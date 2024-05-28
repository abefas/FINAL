#include <stdio.h>
#include <stdlib.h>
#include "header_files/structs.h"

extern int instance_id;

// Function to save clustering results to a CSV file
void save_clustering_to_csv(Point *points, Point *centroids, depot *depots, int num_points, int k) {
    char file_name[50];
    sprintf(file_name, "dbscan-%02d-clusters.csv", instance_id);
    FILE *fp;
    if(NULL == (fp = fopen(file_name, "w")))
    {
        printf("Couldn't open file fp at save_clustering_to_csv\n");
        exit(1);
    }

    // Write header
    fprintf(fp, "Type,X,Y,Cluster\n");

    // Write points
    for (int i = 0; i < num_points; i++) {
        fprintf(fp, "Point,%lf,%lf,%d\n", points[i].x, points[i].y, points[i].cluster);
    }

    // Write centroids
    for (int j = 0; j < k; j++) {
        fprintf(fp, "Centroid,%lf,%lf,%d\n", centroids[j].x, centroids[j].y, j);
    }

    // Write depots
    for (int j = 0; j < k; j++) {
        fprintf(fp, "Depot,%lf,%lf,%d\n", (double)depots[j].x, (double)depots[j].y, j);
    }

    fclose(fp);
}

// Function to plot clusters using GNU Plot
void plot_clusters(Point *points, Point *centroids, depot *depots, int num_points, int k) {
    FILE *gnuplotPipe = popen("gnuplot -persistent", "w");
    if (gnuplotPipe == NULL) {
        fprintf(stderr, "Error opening pipe to GNU Plot.\n");
        return;
    }

    static int version = 1;
    // Create a formatted file name
    char filename[50];
    sprintf(filename, "dbscan-%02d-v%d.png", instance_id, version);
    version++;

    // Set the output terminal to PNG and specify the output file
    fprintf(gnuplotPipe, "set terminal pngcairo size 1920,1080 enhanced font 'Arial,10' fontscale 1.5\n");
    fprintf(gnuplotPipe, "set output '%s'\n", filename);

    // Set plot title and labels
    fprintf(gnuplotPipe, "set title 'K-means++ Clustering'\n");
    fprintf(gnuplotPipe, "set xlabel 'X'\n");
    fprintf(gnuplotPipe, "set ylabel 'Y'\n");
    fprintf(gnuplotPipe, "set grid\n");
    fprintf(gnuplotPipe, "set key inside spacing 1.5\n");

    // Make the legend background transparent
    fprintf(gnuplotPipe, "set key noopaque\n"); // This ensures the legend background is transparent

    // Define colors for the clusters
    const char *colors[] = {"red", "green", "blue", "magenta", "orange", "cyan", "purple", "brown", "pink", "yellow"};

    // Start plotting
    fprintf(gnuplotPipe, "plot ");

    // Plot customers with different colors for each cluster
    for (int c = 0; c < k; c++) {
        if (c > 0) {
            fprintf(gnuplotPipe, ", ");
        }
        fprintf(gnuplotPipe, "'-' using 1:2 with points pointtype 7 pointsize 2 lc rgb '%s' title 'Cluster %d'", colors[c % 10], c + 1);
    }
    //Plot unassigned customers
    fprintf(gnuplotPipe, ", '-' using 1:2 with points pointtype 7 pointsize 2 lc rgb 'gray' title 'Noise'");

    // Add centroids and depots to the plot
    fprintf(gnuplotPipe, ", '-' using 1:2 with points pointtype 74 pointsize 5 title 'Centroids'");
    fprintf(gnuplotPipe, ", '-' using 1:2 with points pointtype 7 pointsize 3 lc rgb 'gray' title 'Depots'\n");

    // Plot customers for each cluster
    for (int c = 0; c < k; c++) {
        for (int i = 0; i < num_points; i++) {
            if (points[i].cluster == c) {
                fprintf(gnuplotPipe, "%lf %lf\n", points[i].x, points[i].y);
            }
        }
        fprintf(gnuplotPipe, "e\n");
    }

    for (int i = 0; i < num_points; i++) {
        if (points[i].cluster == -2) {
            fprintf(gnuplotPipe, "%lf %lf\n", points[i].x, points[i].y);
        }
    }
    fprintf(gnuplotPipe, "e\n");

    // Plot centroids
    for (int j = 0; j < k; j++) {
        fprintf(gnuplotPipe, "%lf %lf\n", centroids[j].x, centroids[j].y);
    }
    fprintf(gnuplotPipe, "e\n");

    // Plot depots
    for (int j = 0; j < k; j++) {
        fprintf(gnuplotPipe, "%lf %lf\n", (double)depots[j].x, (double)depots[j].y);
    }
    fprintf(gnuplotPipe, "e\n");


    // Close the pipe
    pclose(gnuplotPipe);
}

