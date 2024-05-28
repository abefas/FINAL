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

void calculate_k_distances(Point *points, int num_points, int k, double *distances);

// DBSCAN Functions
void region_query(Point *points, int num_points, int point_index, double eps, int *neighbors, int *num_neighbors) {
    *num_neighbors = 0;
    for (int i = 0; i < num_points; i++) {
        if (euclidean_distance(points[point_index], points[i]) <= eps) {
            neighbors[(*num_neighbors)++] = i;
        }
    }
}

bool expand_cluster(Point *points, int num_points, int *clusters, int cluster_id, int point_index, double eps, int minPts) {
    int *seeds = (int *)malloc(num_points * sizeof(int));
    int num_seeds;
    region_query(points, num_points, point_index, eps, seeds, &num_seeds);

    if (num_seeds < minPts) {
        clusters[point_index] = NOISE;
        free(seeds);
        return false;
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
    return true;
}

void dbscan(Point *points, int num_points, double eps, int minPts) {
    int cluster_id = 0;
    int *clusters = malloc(num_points * sizeof(int));
    for (int i = 0; i < num_points; i++) {
        clusters[i] = UNCLASSIFIED;
    }

    for (int i = 0; i < num_points; i++) {
        if (clusters[i] == UNCLASSIFIED) {
            if(expand_cluster(points, num_points, clusters, cluster_id, i, eps, minPts)){
                cluster_id++;
            }
        }
    }

    for (int i = 0; i < num_points; i++) {
        points[i].cluster = clusters[i];
    }

    free(clusters);
}

int initialize_centroids_from_dbscan(Point *points, Point *centroids, int num_points, int k) {
    int *cluster_representatives = malloc(k * sizeof(int));
    int cluster_count = 0;

    // Initialize representatives as uninitialized
    for (int i = 0; i < k; i++) {
        cluster_representatives[i] = UNCLASSIFIED;
    }

    // Identify representative points for each cluster
    for (int i = 0; i < num_points; i++) {
        int cluster_id = points[i].cluster;
        if (cluster_id != NOISE && cluster_id < k && cluster_representatives[cluster_id] == UNCLASSIFIED) {
            centroids[cluster_id] = points[i];
            cluster_representatives[cluster_id] = cluster_id;
            cluster_count++;
        }
    }

    /*
    // Print initialized centroids from DBSCAN clusters
    printf("Initialized centroids from DBSCAN:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
    }
    */

    free(cluster_representatives);

    return cluster_count;
}

double round_down_to_previous_50(double num) {
    return ((int)(num / 50)) * 50;
}
double round_up_to_next_50(double num) {
    return ((int)(num / 50)) * 50 + 50;
}

ClusterData DBSCAN(SON *G) {

    int num_points = G->n_customers;
    int k = G->n_depots; // Number of centroids/clusters for k-means
    int max_iterations = 100;

    Point *points;
    if(NULL == (points = malloc(num_points * sizeof *points))){
        printf("Error mallocing points[]\n");
    }
    for(int i = 0; i < G->n_customers; i++){
        points[i].x = G->a_customers[i].x;
        points[i].y = G->a_customers[i].y;
        points[i].cluster = UNCLASSIFIED;
    }

    // DBSCAN clustering
    // Determine eps dynamically
    double *k_distances = malloc(num_points * sizeof(double));
    int minPts = 3;
    calculate_k_distances(points, num_points, minPts - 1, k_distances);
    //double eps = round_down_to_previous_50(determine_eps(k_distances, num_points));
    double eps = determine_eps(k_distances, num_points);
    //double eps = round_up_to_next_50(determine_eps(k_distances, num_points));
    free(k_distances);
    printf("Determined eps: %0.2lf\n", eps);

    dbscan(points, num_points, eps, minPts);

    // Initialize centroids using DBSCAN clusters once
    Point *centroids;
    if(NULL == (centroids = malloc(k * sizeof *centroids))){
        printf("Error mallocing centroids[]\n");
    }
    int cluster_count = initialize_centroids_from_dbscan(points, centroids, num_points, k);

    ClusterData result;
    result.points = points;
    result.centroids = centroids;
    result.n_clusters = cluster_count;

    return result;
}


//Determine eps functions

int compare_doubles(const void *a, const void *b) {
    double diff = *(double *)a - *(double *)b;
    if (diff < 0) return -1;
    else if (diff > 0) return 1;
    else return 0;
}

void calculate_k_distances(Point *points, int num_points, int k, double *distances) {
    double **all_distances = malloc(num_points * sizeof(double *));
    for (int i = 0; i < num_points; i++) {
        all_distances[i] = malloc(num_points * sizeof(double));
        for (int j = 0; j < num_points; j++) {
            all_distances[i][j] = euclidean_distance(points[i], points[j]);
        }
    }

    for (int i = 0; i < num_points; i++) {
        qsort(all_distances[i], num_points, sizeof(double), compare_doubles);
        distances[i] = all_distances[i][k];
    }

    for (int i = 0; i < num_points; i++) {
        free(all_distances[i]);
    }
    free(all_distances);
}

void plot_k_distances(double *distances, int num_points) {
    qsort(distances, num_points, sizeof(double), compare_doubles); // Sort the distances

    FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
    if (gnuplotPipe == NULL) {
        fprintf(stderr, "Error opening pipe to GNU Plot.\n");
        return;
    }

    fprintf(gnuplotPipe, "set title 'k-Distance Graph'\n");
    fprintf(gnuplotPipe, "set xlabel 'Points'\n");
    fprintf(gnuplotPipe, "set ylabel 'k-Distance'\n");
    fprintf(gnuplotPipe, "set grid\n");
    fprintf(gnuplotPipe, "plot '-' with linespoints title 'k-Distances'\n");

    for (int i = 0; i < num_points; i++) {
        fprintf(gnuplotPipe, "%d %lf\n", i, distances[i]);
    }
    fprintf(gnuplotPipe, "e\n");

    pclose(gnuplotPipe);
}

double determine_eps(double *distances, int num_points) {
    qsort(distances, num_points, sizeof(double), compare_doubles); // Sort the distances

    double *diffs = malloc((num_points - 1) * sizeof(double));
    for (int i = 1; i < num_points; i++) {
        diffs[i - 1] = distances[i] - distances[i - 1];
    }

    // Calculate the mean and standard deviation of the differences
    double sum = 0.0;
    for (int i = 0; i < num_points - 1; i++) {
        sum += diffs[i];
    }
    double mean = sum / (num_points - 1);

    double sum_sq_diff = 0.0;
    for (int i = 0; i < num_points - 1; i++) {
        sum_sq_diff += pow(diffs[i] - mean, 2);
    }
    double std_dev = sqrt(sum_sq_diff / (num_points - 1));

    // Use a threshold based on mean and standard deviation to detect the elbow point
    double threshold = mean + std_dev;
    int elbow_index = 0;
    for (int i = 0; i < num_points - 1; i++) {
        if (diffs[i] > threshold) {
            elbow_index = i;
            break;
        }
    }

    free(diffs);
    return distances[elbow_index];
}

