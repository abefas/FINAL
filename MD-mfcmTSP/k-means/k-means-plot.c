#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "header_files/structs.h"


// Function to calculate the Euclidean distance between two points
double euclidean_distance(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// Function to initialize centroids using k-means++ algorithm
void initialize_centroids(Point *points, Point *centroids, int num_points, int k) {
    centroids[0] = points[rand() % num_points];
    for (int i = 1; i < k; i++) {
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
        //Assign a random node as the next centroid
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

// Function to assign points to the nearest centroid
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

// Function to update centroids based on current cluster assignments
void update_centroids(Point *points, Point *centroids, int num_points, int k) {
    int *cluster_sizes = calloc(k, sizeof(int));
    Point *new_centroids = calloc(k, sizeof(Point));

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

// K-means++ algorithm implementation
void kmeans_plus_plus(Point *points, Point *centroids, int num_points, int k, int max_iterations) {
    for (int iter = 0; iter < max_iterations; iter++) {
        assign_clusters(points, centroids, num_points, k);
        update_centroids(points, centroids, num_points, k);
    }
}

// Function to assign centroids to the nearest depot ensuring every depot gets assigned
void assign_centroids_to_depots(Point *centroids, Point *depots, int k) {
    bool *centroid_assigned = calloc(k, sizeof(bool));
    bool *depot_assigned = calloc(k, sizeof(bool));

    for (int i = 0; i < k; i++) {
        double min_distance = HUGE_VAL;
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

// Main function
ClusterData k_means(SON *G) {
    //srand(time(NULL));

    int num_points = G->n_customers;
    int k = G->n_depots;
    int max_iterations = 100;

    Point *points, *dep_points;
    if(NULL == (points = malloc(sizeof *points * G->n_customers))){
        printf("Error mallocing points[]\n");
    }
    for(int i = 0; i < G->n_customers; i++){
        points[i].x = G->a_customers[i].x;
        points[i].y = G->a_customers[i].y;
    }

    //Depots
    if(NULL == (dep_points = malloc(sizeof *dep_points * G->n_depots))){
        printf("Error mallocing dep_points[]\n");
    }
    for(int i = 0; i < G->n_depots; i++){
        dep_points[i].x = G->a_depots[i].x;
        dep_points[i].y = G->a_depots[i].y;
    }



    Point *centroids;
    if(NULL == (centroids = malloc(sizeof *centroids * k))){
        printf("Error mallocing centroids[]\n");
        exit(1);
    }

    initialize_centroids(points, centroids, num_points, k);

    kmeans_plus_plus(points, centroids, num_points, k, max_iterations);

    /*
    // Print the results
    printf("Final centroids:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%f, %f)\n", i, centroids[i].x, centroids[i].y);
    }
    */

    /*
    printf("\nPoint assignments:\n");
    for (int i = 0; i < num_points; i++) {
        printf("Point %d (%f, %f) is in cluster %d\n", i+1, points[i].x, points[i].y, points[i].cluster);
    }
    */

    // Assign centroids to the nearest depot ensuring every depot gets assigned
    assign_centroids_to_depots(centroids, dep_points, k);

    /*
    // Print the nearest depot assignment for each centroid
    printf("\nCentroid to Depot Assignments:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d is assigned to Depot %d\n", i, centroids[i].cluster);
    }
    */

    //Assign MY depot clustering to k-means++ clusters

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
    result.points = malloc(num_points * sizeof *result.points);
    result.centroids = malloc(k * sizeof *result.centroids);
    memcpy(result.points, points, num_points * sizeof *points);
    memcpy(result.centroids, centroids, k * sizeof *centroids);

    free(counter);
    free(points);
    free(dep_points);
    free(centroids);

    return result;
}

