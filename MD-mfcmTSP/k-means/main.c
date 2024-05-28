#include "header_files/heuristic_main_functions.h"
#include "header_files/heuristic_misc_functions.h"
#include "header_files/structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <omp.h>

int instance_id;

int main(int argc, char **argv) {

    if(argc != 2){
        printf("Usage: %s <instance_id>\n", argv[0]);
        exit(1);
    }else{
        if( sscanf(argv[1], "%02d", &instance_id) != 1) {
            printf("Invalid instance ID\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Read file and store all info */
    char input[100];
    sprintf(input, "../../../../Instances/Cordeau_mfcmTSP/p%02d.MDmfcmTSP", instance_id);
    FILE *fp = fopen(input, "r");
    if(!fp){
        perror("Error while opening the file.\n");
        exit(1);
    }

    SON G;

    /* Get first line and display data info */
    fscanf(fp, "%d %d %d\n", &G.n_customers, &G.n_depots, &G.n_differentTypes);
    printf("numOfCustomers = %d, numOfDepots = %d, n_differentTypes = %d\n",
           G.n_customers, G.n_depots, G.n_differentTypes);

    /* Initialize arrays of SetOfNodes G */
    G.a_depots = malloc(sizeof *G.a_depots * G.n_depots);
    G.a_customers = malloc(sizeof *G.a_customers * G.n_customers);
    G.n_nodes = G.n_customers + G.n_depots;
    G.a_combined = malloc(sizeof *G.a_combined * G.n_nodes);

    /* Initialize Vehicle Type array */
    VType *VT;
    if(NULL == (VT = malloc(sizeof *VT * G.n_differentTypes))){
        perror("Error mallocing VT\n");
        exit(1);
    }
    for(int i = 0; i < G.n_differentTypes; i++){
        VT[i].IVT = i;
        VT[i].n_vehicles = 0;
        VT[i].n_customers = VT[i].n_depots = 0;
    }
    /* Get capacity of each vehicle type */
    fscanf(fp, "%d %d %d\n", &VT[0].capacity, &VT[1].capacity, &VT[2].capacity);

    /* Get speed of each vehicle type */
    fscanf(fp, "%lf %lf %lf\n", &VT[0].speed, &VT[1].speed, &VT[2].speed);

    /* da_access[ivt] has length n_customers and contains 1 in cell ID-1 if
   * customer can be visited by ivt */
    int **da_access;
    if (NULL == (da_access = malloc(G.n_differentTypes * sizeof *da_access))) {
        perror("Error mallocing da_access!\n");
        exit(1);
    }
    for (int i = 0; i < G.n_differentTypes; i++) {
        if (NULL == (da_access[i] = calloc(G.n_nodes, sizeof *da_access[i]))) {
            perror("Error callocing da_access[i]!\n");
            exit(1);
        }
    }

    /* Get customers info and store in SetOfNodes G */
    int accessibility;
    for (int i = 0; i < G.n_customers; i++) {
        fscanf(fp, "%d %f %f %d %d\n", &G.a_customers[i].id, &G.a_customers[i].x,
               &G.a_customers[i].y, &G.a_customers[i].demand, &accessibility);

        // CHECK ORDER OF VEHICLES FOR ACCESSIBILITY
        while (accessibility > 0) {
            int a = accessibility % 10;
            VT[a - 1].n_customers++;
            da_access[a - 1][i] = 1;
            accessibility /= 10;
        }
        G.a_combined[i].id = G.a_customers[i].id;
        G.a_combined[i].x = G.a_customers[i].x;
        G.a_combined[i].y = G.a_customers[i].y;
    }

    /* Get depots info */
    for (int i = 0; i < G.n_depots; i++) {
        if (NULL == (G.a_depots[i].n_VT =
            malloc(sizeof *G.a_depots[i].n_VT * G.n_differentTypes))) {
            perror("Error mallocing n_VT!\n");
            exit(1);
        }
        fscanf(fp, "%d %f %f %d %d %d\n", &G.a_depots[i].id, &G.a_depots[i].x,
               &G.a_depots[i].y, &G.a_depots[i].n_VT[0], &G.a_depots[i].n_VT[1],
               &G.a_depots[i].n_VT[2]);

        for (int ivt = 0; ivt < G.n_differentTypes; ivt++) {
            /* Get number of vehicles of this type */
            VT[ivt].n_vehicles += G.a_depots[i].n_VT[ivt];

            /* Get number of depots for each Vehicle Type */
            if (G.a_depots[i].n_VT[ivt] != 0){
                VT[ivt].n_depots++;
                da_access[ivt][G.a_depots[i].id - 1] = 1;
            }
        }

        G.a_combined[G.n_customers + i].id = G.a_depots[i].id;
        G.a_combined[G.n_customers + i].x = G.a_depots[i].x;
        G.a_combined[G.n_customers + i].y = G.a_depots[i].y;
    }

    int check = fclose(fp);
    if (check != 0) {
        perror("fclose error at main\n");
        exit(1);
    }

    if(NULL == (G.d_matrix = malloc(sizeof *G.d_matrix * G.n_nodes))){
        printf("Error mallocing d_matrix!\n");
        exit(1);
    }
    for(int i = 0; i < G.n_nodes; i++){
        if(NULL == (G.d_matrix[i] = malloc(sizeof *G.d_matrix[i] * G.n_nodes))){
            printf("Error mallocing d_matrix[]!\n");
            exit(1);
        }
    }

    for(int i = 0; i < G.n_nodes; i++){
        for(int j = 0; j < G.n_nodes; j++){
            if(i == j || (i >= G.n_customers && j >= G.n_customers)){
                G.d_matrix[i][j] = 0.0;
            }else{
                G.d_matrix[i][j] = sqrt(pow(G.a_combined[i].x - G.a_combined[j].x, 2) + pow(G.a_combined[i].y - G.a_combined[j].y, 2));
            }

        }
    }

    srand(time(NULL));
    asolution R;
    R.total_makespan = HUGE_VAL;
    if(NULL == (R.a_VT = malloc(sizeof *R.a_VT * G.n_differentTypes))){
        printf("Error mallocing R.a_VT in heuristic_prox.c\n");
        exit(1);
    }
    for(int ivt = 0; ivt < G.n_differentTypes; ivt++){
        if(NULL == (R.a_VT[ivt].a_depots = malloc(sizeof *R.a_VT[ivt].a_depots * G.n_depots))){
            printf("Error mallocing R.a_VT[%d].a_depots", ivt);
            exit(1);
        }
        R.a_VT[ivt].makespan = 0.0;
        R.a_VT[ivt].IVT = ivt;
        for(int idep = 0; idep < G.n_depots; idep++){
            R.a_VT[ivt].a_depots[idep].routelist = NULL;
            R.a_VT[ivt].a_depots[idep].quantity_served = 0;
            R.a_VT[ivt].a_depots[idep].depot_id = G.a_depots[idep].id;
            if(G.a_depots[idep].n_VT[ivt] == 0){
                R.a_VT[ivt].a_depots[idep].makespan = -1;
            }else{
                R.a_VT[ivt].a_depots[idep].makespan = 0.0;
            }
        }
    }

    plotClusters pl;
    pl.points = malloc(G.n_customers * sizeof *pl.points);
    pl.centroids = malloc(G.n_depots * sizeof *pl.centroids);

    time_t begin = time(NULL);

    //Run Algorithm
    #pragma omp parallel for
    for(int i = 0; i < 20 * G.n_customers; i++){
        asolution R_local;
        R_local.total_makespan = HUGE_VAL;
        if(NULL == (R_local.a_VT = malloc(sizeof *R_local.a_VT * G.n_differentTypes))){
            printf("Error mallocing R.a_VT in heuristic_prox.c\n");
            exit(1);
        }
        for(int ivt = 0; ivt < G.n_differentTypes; ivt++){
            if(NULL == (R_local.a_VT[ivt].a_depots = malloc(sizeof *R_local.a_VT[ivt].a_depots * G.n_depots))){
                printf("Error mallocing R.a_VT[%d].a_depots", ivt);
                exit(1);
            }
            R_local.a_VT[ivt].makespan = 0.0;
            R_local.a_VT[ivt].IVT = ivt;
            for(int idep = 0; idep < G.n_depots; idep++){
                R_local.a_VT[ivt].a_depots[idep].routelist = NULL;
                R_local.a_VT[ivt].a_depots[idep].quantity_served = 0;
                R_local.a_VT[ivt].a_depots[idep].depot_id = G.a_depots[idep].id;
                if(G.a_depots[idep].n_VT[ivt] == 0){
                    R_local.a_VT[ivt].a_depots[idep].makespan = -1;
                }else{
                    R_local.a_VT[ivt].a_depots[idep].makespan = 0.0;
                }
            }
        }
        plotClusters pl_local;
        pl_local.points = malloc(G.n_customers * sizeof *pl_local.points);
        pl_local.centroids = malloc(G.n_depots * sizeof *pl_local.centroids);

        heuristic(&G, VT, da_access, &R_local, &pl_local);

        #pragma omp critical
        {
            if(R_local.total_makespan < R.total_makespan){
                memcpy(pl.points, pl_local.points, G.n_customers * sizeof *pl.points);
                memcpy(pl.centroids, pl_local.centroids, G.n_depots * sizeof *pl.centroids);
                for(int ivt = 0; ivt < G.n_differentTypes; ivt++){
                    for(int idep = 0; idep < G.n_depots; idep++){
                        deleteList(&R.a_VT[ivt].a_depots[idep].routelist);
                        R.a_VT[ivt].a_depots[idep].routelist = copyList(R_local.a_VT[ivt].a_depots[idep].routelist);
                        R.a_VT[ivt].a_depots[idep].makespan = R_local.a_VT[ivt].a_depots[idep].makespan;
                    }
                    R.a_VT[ivt].makespan = R_local.a_VT[ivt].makespan;
                }
                R.total_makespan = R_local.total_makespan;
            }
        }

        //Free local vars
        for(int ivt = 0; ivt < G.n_differentTypes; ivt++){
            for(int idep = 0; idep < G.n_depots; idep++){
                deleteList(&R_local.a_VT[ivt].a_depots[idep].routelist);
            }
            free(R_local.a_VT[ivt].a_depots);
        }
        free(R_local.a_VT);
        free(pl_local.centroids);
        free(pl_local.points);

    }

    time_t finish = time(NULL);
    double runtime = difftime(finish, begin);

    fprint_results(&R, &G, VT);
    fprint_data(runtime);
    plot_clusters(pl.points, pl.centroids, G.a_depots, G.n_customers, G.n_depots);
    save_clustering_to_csv(pl.points, pl.centroids, G.a_depots, G.n_customers, G.n_depots);

    free(pl.centroids);
    free(pl.points);

    for(int ivt = 0; ivt < G.n_differentTypes; ivt++){
        for(int idep = 0; idep < G.n_depots; idep++){
            deleteList(&R.a_VT[ivt].a_depots[idep].routelist);
        }
        free(R.a_VT[ivt].a_depots);
    }
    free(R.a_VT);


    for (int i = 0; i < G.n_differentTypes; i++) {
        free(da_access[i]);
    }
    free(da_access);

    for(int i = 0; i < G.n_nodes; i++){
        free(G.d_matrix[i]);
    }
    free(G.d_matrix);

    for (int i = 0; i < G.n_depots; i++) {
        free(G.a_depots[i].n_VT);
    }

    free(VT);
    free(G.a_customers);
    free(G.a_depots);
    free(G.a_combined);

    return 0;
}
