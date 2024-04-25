#include "header_files/heuristic_main_functions.h"
#include "header_files/structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int instance_id;

int main(int argc, char **argv) {

    if(argc != 2){
        printf("Usage: %s <version_number>\n", argv[0]);
        exit(1);
    }else{
        if( sscanf(argv[1], "%02d", &instance_id) != 1) {
            printf("Invalid instance ID\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Read file and store all info */
    FILE *fp = fopen("../../../Instances/i500/i500_01.MD-mfcmTSP", "r");
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
        fscanf(fp, "%d %d %d %d %d\n", &G.a_customers[i].id, &G.a_customers[i].x,
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
        fscanf(fp, "%d %d %d %d %d %d\n", &G.a_depots[i].id, &G.a_depots[i].x,
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


    heuristic_prox(&G, VT, da_access);


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
