#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include "header_files/heuristic_main_functions.h"
#include "header_files/structs.h"

#define UNCLASSIFIED -1
#define NOISE -2

typedef struct {
    double x;
    double y;
    int cluster; // This will be set during clustering
} Point;

void calculate_k_distances(Point *points, int num_points, int k, double *distances);

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
    int *clusters = (int *)malloc(num_points * sizeof(int));
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

void plot_clusters(Point *points, Point *centroids, Point *depots, int num_points, int n_depots, int n_clusters) {
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
    fprintf(gnuplotPipe, "plot '-' using 1:2:3 with points palette pointtype 7 pointsize 1 title 'Data Points', '-' using 1:2 with points pointtype 4 pointsize 2 lc rgb 'black' title 'Centroids', '-' using 1:2 with points pointtype 5 pointsize 2 lc rgb 'gray' title 'Depots'\n");

    // Plot data points
    //fprintf(gnuplotPipe, "plot '-' using 1:2:3 with points palette pointtype 7 pointsize 1 title 'Data Points', '-' using 1:2 with points pointtype 4 pointsize 2 lc rgb 'black' title 'Centroids'\n");

    // Data points
    for (int i = 0; i < num_points; i++) {
        fprintf(gnuplotPipe, "%lf %lf %d\n", points[i].x, points[i].y, points[i].cluster);
    }
    fprintf(gnuplotPipe, "e\n");
    // Centroids
    for (int j = 0; j < n_clusters; j++) {
        fprintf(gnuplotPipe, "%lf %lf\n", centroids[j].x, centroids[j].y);
    }
    fprintf(gnuplotPipe, "e\n");

    // Plot depots
    for (int j = 0; j < n_depots; j++) {
        fprintf(gnuplotPipe, "%lf %lf\n", depots[j].x, depots[j].y);
    }
    fprintf(gnuplotPipe, "e\n");


    pclose(gnuplotPipe);
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

void assign_clusters_to_depots(Point *points, Point *centroids, Point *depots, int num_points, int num_clusters, int num_depots) {
    for (int i = 0; i < num_clusters; i++) {
        double min_distance = DBL_MAX;
        int nearest_depot = -1;
        for (int j = 0; j < num_depots; j++) {
            double distance = euclidean_distance(centroids[i], depots[j]);
            if (distance < min_distance) {
                min_distance = distance;
                nearest_depot = j;
            }
        }
        if (nearest_depot != -1) {
            centroids[i].cluster = nearest_depot;
        }
    }

    // Assign points to the depot clusters
    for (int i = 0; i < num_points; i++) {
        if (points[i].cluster != NOISE) {
            points[i].cluster = centroids[points[i].cluster].cluster;
        }
    }
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

double calculate_sse(Point *points, Point *centroids, int num_points, int k) {
    double sse = 0.0;
    for (int i = 0; i < num_points; i++) {
        int cluster = points[i].cluster;
        double distance = euclidean_distance(points[i], centroids[cluster]);
        sse += distance * distance;
    }
    return sse;
}

double round_down_to_previous_50(double num) {
    return ((int)(num / 50)) * 50;
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

    //Depots 
    if(NULL == (dep_points = malloc(sizeof *dep_points * G->n_depots))){
        printf("Error mallocing dep_points[]\n");
    }
    for(int i = 0; i < G->n_depots; i++){
        dep_points[i].x = G->a_depots[i].x;
        dep_points[i].y = G->a_depots[i].y;
    }

    // DBSCAN clustering
    // Determine eps dynamically
    double *k_distances = malloc(num_points * sizeof(double));
    int minPts = 3;
    calculate_k_distances(points, num_points, minPts - 1, k_distances);
    double eps = round_down_to_previous_50(determine_eps(k_distances, num_points));
    free(k_distances);
    printf("Determined eps: %0.2lf\n", eps);

    dbscan(points, num_points, eps, minPts);


    // Run k-means multiple times and select the best clustering based on SSE
    double best_sse = DBL_MAX;
    Point *best_centroids = malloc(k * sizeof(Point));

    // Initialize centroids using DBSCAN clusters once
    Point *initial_centroids = malloc(k * sizeof(Point));
    int cluster_count = initialize_centroids_from_dbscan(points, initial_centroids, num_points, k);

    plot_clusters(points, best_centroids, dep_points, num_points, G->n_depots, k);
    for (int run = 0; run < 10; run++) {
        Point *centroids = malloc(k * sizeof(Point));
        memcpy(centroids, initial_centroids, k * sizeof(Point));

        // If fewer clusters found than required, initialize the remaining centroids using k-means++
        if (cluster_count < k) {
            initialize_remaining_centroids_kmeanspp(points, centroids, num_points, k, cluster_count);
        }
        plot_clusters(points, best_centroids, dep_points, num_points, G->n_depots, k);

        // Run k-means
        kmeans(points, centroids, num_points, k, max_iterations);

        /*
        // Print final centroids after k-means
        printf("Final centroids after k-means:\n");
        for (int i = 0; i < k; i++) {
            printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
        }
        */

        // Calculate SSE for this run
        double sse = calculate_sse(points, centroids, num_points, k);
        if (sse < best_sse) {
            best_sse = sse;
            memcpy(best_centroids, centroids, k * sizeof(Point));
        }

        free(centroids);
    }

    free(initial_centroids);


    /*
    // Initialize centroids for k-means++ using DBSCAN clusters
    Point *centroids;
    if(NULL == (centroids = malloc(sizeof *centroids * k))){
        printf("Error mallocing centroids[]\n");
        exit(1);
    }
    initialize_centroids_from_dbscan(points, centroids, num_points, k);
    */

    /*
    // Print all centroids before running k-means
    printf("All centroids before running k-means:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, best_centroids[i].x, best_centroids[i].y);
    }
    */

    // K-means clustering to refine clusters
    //kmeans(points, best_centroids, num_points, k, max_iterations);

    /*
    // Print final centroids after k-means
    printf("Final centroids after k-means:\n");
    for (int i = 0; i < k; i++) {
        printf("Centroid %d: (%lf, %lf)\n", i, centroids[i].x, centroids[i].y);
    }
    */

    // Assign noise points to the nearest k-means centroids
    assign_noise_points(points, best_centroids, num_points, k);

    /*
    // Print final results after assigning noise points
    printf("\nFinal Results After Assigning Noise Points:\n");
    for (int i = 0; i < num_points; i++) {
        printf("Point %d (%lf, %lf) is in cluster %d\n", i, points[i].x, points[i].y, points[i].cluster);
    }
    */

    //Assign centroids to the nearest depot ensuring every depot get assigned
    assign_centroids_to_depots(best_centroids, dep_points, k);

    // Plot final clustering result
    //plot_clusters(points, best_centroids, dep_points, num_points, G->n_depots, k);


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
        int idepot = best_centroids[points[i].cluster].cluster;
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
        int idepot = best_centroids[points[i].cluster].cluster;
        cluster[idepot][counter[idepot]] = i+1;
        counter[idepot]++;
    }


    ClusterData result;
    result.cluster = cluster;
    result.limit = limit;

    free(counter);
    free(points);
    free(dep_points);
    free(best_centroids);

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

    int elbow_index = num_points / 2; // A simple heuristic for elbow detection
    for (int i = 1; i < num_points - 1; i++) {
        if ((distances[i] - distances[i - 1]) < (distances[i + 1] - distances[i])) {
            elbow_index = i;
            break;
        }
    }

    return distances[elbow_index];
}
