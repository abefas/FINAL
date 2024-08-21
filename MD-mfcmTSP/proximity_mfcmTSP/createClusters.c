#include <stdlib.h>
#include "header_files/heuristic_misc_functions.h"
#include "header_files/structs.h"

//void save_clustering_to_csv(SON *G, int *assign_cluster);

ClusterData createClusters(SON *G, int **da_access){

    int *assign_cluster;
    if(NULL == (assign_cluster = malloc(sizeof *assign_cluster * G->n_customers))){
        printf("Error mallocing assign_cluster!\n");
        exit(1);
    }

    for(int i = 0; i < G->n_customers; i++){
        int depotID = find_closest_depot(i+1, G, da_access);
        assign_cluster[i] = depotID;
    }

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
        int idepot = assign_cluster[i] - G->n_customers - 1;
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

    for(int i = 0; i < G->n_customers; i++){
        int idepot = assign_cluster[i] - G->n_customers - 1;
        cluster[idepot][counter[idepot]] = i+1;
        counter[idepot]++;
    }
    //save_clustering_to_csv(G, assign_cluster);
    free(counter);
    free(assign_cluster);

    ClusterData result;
    result.cluster = cluster;
    result.limit = limit;
    
    return result;
}

/*
extern int instance_id;
// Function to save clustering results to a CSV file
void save_clustering_to_csv(SON *G, int *assign_cluster) {
    char file_name[20];
    sprintf(file_name, "prox-%02d.clusters", instance_id);
    FILE *fp;
    if(NULL == (fp = fopen(file_name, "w")))
    {
        printf("Couldn't open file fp at save_clustering_to_csv\n");
        exit(1);
    }

    // Write header
    fprintf(fp, "Type,X,Y,Cluster\n");

    // Write points
    for (int i = 0; i < G->n_customers; i++) {
        fprintf(fp, "Point,%f,%f,%d\n", G->a_customers[i].x, G->a_customers[i].y, assign_cluster[i]-G->n_customers-1);
    }

    // Write centroids
    for (int j = 0; j < G->n_depots; j++) {
        fprintf(fp, "Centroid,%f,%f,%d\n", G->a_depots[j].x, G->a_depots[j].y, j);
    }

    // Write depots
    for (int j = 0; j < G->n_depots; j++) {
        fprintf(fp, "Depot,%f,%f,%d\n", G->a_depots[j].x, G->a_depots[j].y, j);
    }

    fclose(fp);
}
*/

