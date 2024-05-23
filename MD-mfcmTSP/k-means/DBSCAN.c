#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include "header_files/structs.h"

#define UNCLASSIFIED -1
#define NOISE -2

typedef struct {
    double x;
    double y;
    int cluster; // This will be set during clustering
} Point;

double euclidean_distance(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// DBSCAN Functions
void region_query(Point *points, int num_points, int point_index, double eps, int *neighbors, int *num_neighbors) {
    *num_neighbors = 0;
    for (int i = 0; i < num_points; i++) {
        if (euclidean_distance(points[point_index], points[i]) <= eps) {
            neighbors[(*num_neighbors)++] = i;
        }
    }
}

void expand_cluster(Point *points, int num_points, int *clusters, int cluster_id, int point_index, double eps, int minPts) {
    int *seeds = (int *)malloc(num_points * sizeof(int));
    int num_seeds;
    region_query(points, num_points, point_index, eps, seeds, &num_seeds);

    if (num_seeds < minPts) {
        clusters[point_index] = NOISE;
        free(seeds);
        return;
    }

    for (int i = 0; i < num_seeds; i++) {
        clusters[seeds[i]] = cluster_id;
    }

    int current_point_index = 0;
    while (current_point_index < num_seeds) {
        int current_point = seeds[current_point_index];
        int *result = (int *)malloc(num_points * sizeof(int));
        int result_size;
        region_query(points, num_points, current_point, eps, result, &result_size);

        if (result_size >= minPts) {
            for (int i = 0; i < result_size; i++) {
                int result_point = result[i];
                if (clusters[result_point] == UNCLASSIFIED || clusters[result_point] == NOISE) {
                    if (clusters[result_point] == UNCLASSIFIED) {
                        seeds[num_seeds++] = result_point;
                    }
                    clusters[result_point] = cluster_id;
                }
            }
        }

        free(result);
        current_point_index++;
    }

    free(seeds);
}

void dbscan(Point *points, int num_points, double eps, int minPts) {
    int cluster_id = 0;
    int *clusters = (int *)malloc(num_points * sizeof(int));
    for (int i = 0; i < num_points; i++) {
        clusters[i] = UNCLASSIFIED;
    }

    for (int i = 0; i < num_points; i++) {
        if (clusters[i] == UNCLASSIFIED) {
            expand_cluster(points, num_points, clusters, cluster_id, i, eps, minPts);
            cluster_id++;
        }
    }

    for (int i = 0; i < num_points; i++) {
        points[i].cluster = clusters[i];
    }

    free(clusters);
}

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

// K-means Functions
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

void kmeans(Point *points, Point *centroids, int num_points, int k, int max_iterations) {
    for (int iter = 0; iter < max_iterations; iter++) {
        assign_clusters(points, centroids, num_points, k);
        update_centroids(points, centroids, num_points, k);
    }
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

void plot_clusters(Point *points, Point *centroids, int num_points, int k) {
    FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
    if (gnuplotPipe == NULL) {
        fprintf(stderr, "Error opening pipe to GNU Plot.\n");
        return;
    }

    fprintf(gnuplotPipe, "set title 'Clustering Result'\n");
    fprintf(gnuplotPipe, "set xlabel 'X'\n");
    fprintf(gnuplotPipe, "set ylabel 'Y'\n");
    fprintf(gnuplotPipe, "set grid\n");
    fprintf(gnuplotPipe, "set key outside\n");
    fprintf(gnuplotPipe, "set palette model RGB defined (0 'red', 1 'green', 2 'blue', 3 'yellow', 4 'magenta', 5 'cyan')\n");

    // Plot data points
    fprintf(gnuplotPipe, "plot '-' using 1:2:3 with points palette pointtype 7 pointsize 1 title 'Data Points', '-' using 1:2 with points pointtype 4 pointsize 2 lc rgb 'black' title 'Centroids'\n");

    // Data points
    for (int i = 0; i < num_points; i++) {
        fprintf(gnuplotPipe, "%lf %lf %d\n", points[i].x, points[i].y, points[i].cluster);
    }
    fprintf(gnuplotPipe, "e\n");

    // Centroids
    for (int j = 0; j < k; j++) {
        fprintf(gnuplotPipe, "%lf %lf\n", centroids[j].x, centroids[j].y);
    }
    fprintf(gnuplotPipe, "e\n");

    pclose(gnuplotPipe);
}

ClusterData DBSCAN_plus_kmeans(SON *G) {

    int num_points = G->n_customers;
    int k = G->n_depots; // Number of centroids/clusters for k-means
    int max_iterations = 100;

    Point *points, *dep_points;
    if(NULL == (points = malloc(sizeof *points * G->n_customers))){
        printf("Error mallocing points[]\n");
    }
    for(int i = 0; i < G->n_customers; i++){
        points[i].x = G->a_customers[i].x;
        points[i].y = G->a_customers[i].y;
        points[i].cluster = UNCLASSIFIED;
    }

    //Depots for visualization only
    if(NULL == (dep_points = malloc(sizeof *dep_points * G->n_depots))){
        printf("Error mallocing dep_points[]\n");
    }
    for(int i = 0; i < G->n_depots; i++){
        dep_points[i].x = G->a_depots[i].x;
        dep_points[i].y = G->a_depots[i].y;
    }

    // DBSCAN clustering
    double eps = 1.5;
    int minPts = 3;
    dbscan(points, num_points, eps, minPts);

    // Print initial DBSCAN results
    printf("Initial DBSCAN Results:\n");
    for (int i = 0; i < num_points; i++) {
        if (points[i].cluster == NOISE) {
            printf("Point %d (%lf, %lf) is noise\n", i, points[i].x, points[i].y);
        } else {
            printf("Point %d (%lf, %lf) is in cluster %d\n", i, points[i].x, points[i].y, points[i].cluster);
        }
    }

    // Initialize centroids for k-means++ using DBSCAN clusters
    Point *centroids;
    if(NULL == (centroids = malloc(sizeof *centroids * k))){
        printf("Error mallocing centroids[]\n");
        exit(1);
    }
    int cluster_count = 0;
    for (int i = 0; i < num_points; i++) {
        if (points[i].cluster != NOISE && cluster_count < k) {
            centroids[cluster_count++] = points[i];
        }
    }

    // Print initialized centroids from DBSCAN clusters
    printf("Initialized centroids from DBSCAN:\n");
    for (int i = 0; i < cluster_count; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
    }

    // If DBSCAN found fewer clusters than k, initialize remaining centroids with k-means++
    if (cluster_count < k) {
        printf("Initializing remaining centroids with k-means++...\n");
        initialize_remaining_centroids_kmeanspp(points, centroids, num_points, k, cluster_count);
    }

    // Print all centroids before running k-means
    printf("All centroids before running k-means:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
    }

    // K-means clustering to refine clusters
    kmeans(points, centroids, num_points, k, max_iterations);

    // Print final centroids after k-means
    printf("Final centroids after k-means:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
    }

    // Assign noise points to the nearest k-means centroids
    assign_noise_points(points, centroids, num_points, k);

    // Print final results after assigning noise points
    printf("\nFinal Results After Assigning Noise Points:\n");
    for (int i = 0; i < num_points; i++) {
        printf("Point %d (%lf, %lf) is in cluster %d\n", i, points[i].x, points[i].y, points[i].cluster);
    }

    // Plot final clustering result
    plot_clusters(points, centroids, num_points, k);

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

    free(counter);
    free(points);
    free(dep_points);
    free(centroids);

    return result;
}

