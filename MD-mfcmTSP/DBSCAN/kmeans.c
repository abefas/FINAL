#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include "header_files/heuristic_main_functions.h"
#include "header_files/heuristic_misc_functions.h"
#include "header_files/structs.h"

#define UNCLASSIFIED -1
#define NOISE -2

// K-means Functions
void initialize_remaining_centroids_kmeanspp(Point *points, Point *centroids, int num_points, int k, int initialized) {
    for (int i = initialized; i < k; i++) {
        double *distances = malloc(num_points * sizeof(double));
        double total_distance = 0.0;
        for (int j = 0; j < num_points; j++) {
            double min_distance = DBL_MAX;
            for (int m = 0; m < i; m++) {
                double dist = euclidean_distance(points[j], centroids[m]);
                if (dist < min_distance) {
                    min_distance = dist;
                }
            }
            distances[j] = min_distance;
            total_distance += min_distance;
        }
        double r = ((double) rand() / (RAND_MAX)) * total_distance;
        for (int j = 0; j < num_points; j++) {
            r -= distances[j];
            if (r <= 0) {
                centroids[i] = points[j];
                break;
            }
        }
        free(distances);
    }
}

void assign_clusters(Point *points, Point *centroids, int num_points, int k) {
    for (int i = 0; i < num_points; i++) {
        double min_distance = DBL_MAX;
        int cluster = 0;
        for (int j = 0; j < k; j++) {
            double distance = euclidean_distance(points[i], centroids[j]);
            if (distance < min_distance) {
                min_distance = distance;
                cluster = j;
            }
        }
        points[i].cluster = cluster;
    }
}

void update_centroids(Point *points, Point *centroids, int num_points, int k) {
    int *cluster_sizes = calloc(k, sizeof *cluster_sizes);
    Point *new_centroids = calloc(k, sizeof *new_centroids);

    for (int i = 0; i < num_points; i++) {
        int cluster = points[i].cluster;
        new_centroids[cluster].x += points[i].x;
        new_centroids[cluster].y += points[i].y;
        cluster_sizes[cluster]++;
    }

    for (int j = 0; j < k; j++) {
        if (cluster_sizes[j] > 0) {
            centroids[j].x = new_centroids[j].x / cluster_sizes[j];
            centroids[j].y = new_centroids[j].y / cluster_sizes[j];
        }
    }

    free(cluster_sizes);
    free(new_centroids);
}

int find_nearest_cluster(Point *centroids, int k, Point noise_point) {
    double min_distance = DBL_MAX;
    int nearest_cluster = -1;
    for (int i = 0; i < k; i++) {
        double distance = euclidean_distance(noise_point, centroids[i]);
        if (distance < min_distance) {
            min_distance = distance;
            nearest_cluster = i;
        }
    }
    return nearest_cluster;
}

void assign_noise_points(Point *points, Point *centroids, int num_points, int k) {
    for (int i = 0; i < num_points; i++) {
        if (points[i].cluster == NOISE) {
            int nearest_cluster = find_nearest_cluster(centroids, k, points[i]);
            points[i].cluster = nearest_cluster;
        }
    }
}

// Function to assign centroids to the nearest depot ensuring every depot gets assigned
void assign_centroids_to_depots(Point *centroids, Point *depots, int k) {
    bool *centroid_assigned = calloc(k, sizeof(bool));
    bool *depot_assigned = calloc(k, sizeof(bool));

    for (int i = 0; i < k; i++) {
        double min_distance = DBL_MAX;
        int nearest_depot = -1;
        int nearest_centroid = -1;

        // Find the nearest unassigned centroid-depot pair
        for (int j = 0; j < k; j++) {
            if (!depot_assigned[j]) {
                for (int m = 0; m < k; m++) {
                    if (!centroid_assigned[m]) {
                        double distance = euclidean_distance(centroids[m], depots[j]);
                        if (distance < min_distance) {
                            min_distance = distance;
                            nearest_depot = j;
                            nearest_centroid = m;
                        }
                    }
                }
            }
        }

        // Assign the nearest unassigned centroid to the nearest unassigned depot
        if (nearest_depot != -1 && nearest_centroid != -1) {
            centroids[nearest_centroid].cluster = nearest_depot;
            depot_assigned[nearest_depot] = true;
            centroid_assigned[nearest_centroid] = true;
        }
    }

    free(centroid_assigned);
    free(depot_assigned);
}

ClusterData kmeans(SON *G, ClusterData *dbscan) {

    int num_points = G->n_customers;
    int k = G->n_depots; // Number of centroids/clusters for k-means
    int max_iterations = 100;

    Point *points, *centroids;
    if(NULL == (points = malloc(num_points * sizeof *points))){
        printf("Error mallocing points[]\n");
    }
    if(NULL == (centroids = malloc(k * sizeof *centroids))){
        printf("Error mallocing centroids[]\n");
    }
    memcpy(points, dbscan->points, num_points * sizeof *points);
    memcpy(centroids, dbscan->centroids, k * sizeof *centroids);

    // If fewer clusters found than required, initialize the remaining centroids using k-means++
    if (dbscan->n_clusters < k) {
        initialize_remaining_centroids_kmeanspp(points, centroids, num_points, k, dbscan->n_clusters);
    }

    // Run k-means
    for (int iter = 0; iter < max_iterations; iter++) {
        assign_clusters(points, centroids, num_points, k);
        update_centroids(points, centroids, num_points, k);
    }

    /*
    // Print final centroids after k-means
    printf("Final centroids after k-means:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
    }
    */

    // Assign noise points to the nearest k-means centroids
    assign_noise_points(points, centroids, num_points, k);

    /*
    // Print final results after assigning noise points
    printf("\nFinal Results After Assigning Noise Points:\n");
    for (int i = 0; i < num_points; i++) {
        printf("Point %d (%lf, %lf) is in cluster %d\n", i, points[i].x, points[i].y, points[i].cluster);
    }
    */

    Point *dep_points;
    //Depots
    if(NULL == (dep_points = malloc(sizeof *dep_points * G->n_depots))){
        printf("Error mallocing dep_points[]\n");
    }
    for(int i = 0; i < G->n_depots; i++){
        dep_points[i].x = G->a_depots[i].x;
        dep_points[i].y = G->a_depots[i].y;
    }

    //Assign centroids to the nearest depot ensuring every depot get assigned
    assign_centroids_to_depots(centroids, dep_points, k);


    //Assign MY depot clustering to clusters created
    int **cluster, *counter, *limit;
    if(NULL == (cluster = malloc(sizeof *cluster * G->n_depots))){
        printf("Error mallocing **cluster!\n");
        exit(1);
    }
    if(NULL == (counter = calloc(G->n_depots, sizeof *counter))){
        printf("Error mallocing counter!\n");
        exit(1);
    }
    if(NULL == (limit = calloc(G->n_depots, sizeof *limit))){
        printf("Error mallocing limit!\n");
        exit(1);
    }
    for(int i = 0; i < G->n_customers; i++){
        int idepot = centroids[points[i].cluster].cluster;
        if(idepot < 0){
            printf("Customer Cluster %d\nCustomer %d idepot %d\n", points[i].cluster, i+1, idepot);
            exit(1);
        }
        counter[idepot]++;
    }
    for(int dep = 0; dep < G->n_depots; dep++){
        if(NULL == (cluster[dep] = calloc(counter[dep], sizeof *cluster[dep]))){
            printf("Error mallocing *cluster[dep]!\n");
            exit(1);
        }
        limit[dep] = counter[dep];
        counter[dep] = 0;
    }
    //What we have
    //points[i].cluster == cluster that customer has been assigned to 
    //centroids[i].cluster == depot that cluster has been assigned to 
    for(int i = 0; i < G->n_customers; i++){
        int idepot = centroids[points[i].cluster].cluster;
        cluster[idepot][counter[idepot]] = i+1;
        counter[idepot]++;
    }


    ClusterData result;
    result.cluster = cluster;
    result.limit = limit;
    result.points = points;
    result.centroids = centroids;

    free(counter);
    free(dep_points);

    return result;
}
