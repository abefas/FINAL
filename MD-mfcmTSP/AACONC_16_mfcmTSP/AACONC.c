#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"

void AACONC(SON *G, VType *VT, int **da_access, int n_ants, int n_freq, int n_size, int n_sect, int n_prim, 
             double T_update, double a_update, double p_min, double p_max, double d, double a, double b)
{

/* Initialize 4-D pheromone array */
    double *phMatrix;
    if(NULL == (phMatrix = malloc(sizeof *phMatrix * G->n_depots*G->n_differentTypes*G->n_nodes*G->n_nodes))){
        printf("Error mallocing phMatrix!\n");
        exit(1);
    }
    for(int d = 0; d < G->n_depots; d++){
        for(int t = 0; t < G->n_differentTypes; t++){
            for(int k = 0; k < G->n_nodes; k++){
                for(int l = 0; l < G->n_nodes; l++){
                    //node to itself OR depot to depot or Depot doesn't support vehicle type
                    if(k == l || (k >= G->n_customers && l >= G->n_customers)){   
                        phMatrix[d*G->n_differentTypes*G->n_nodes*G->n_nodes + t*G->n_nodes*G->n_nodes + k*G->n_nodes + l] = -1.0;
                    }else{
                        phMatrix[d*G->n_differentTypes*G->n_nodes*G->n_nodes + t*G->n_nodes*G->n_nodes + k*G->n_nodes + l] = 1.0;
                    }
                }
            }
        }
    }

    /* Clusters for each vehicle type and each node (Customer and Depot) */
    int ***K = malloc(sizeof *K * (G->n_differentTypes));
    int rows = (int)ceil((double)G->n_customers/n_size);
    int counter = 0;

    /* Cluster creation takes into consideration customer accessibility */
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        K[ivt] = malloc(sizeof *K[ivt] * (G->n_nodes));
        if(ivt == 2){
            //Drone cannot move from customer to customer 
            for(int i = 0; i < G->n_customers; i++){
                K[ivt][i] = calloc(rows * n_size, sizeof *K[ivt][i]);
                if(!K[ivt][i]){perror("Error callocing K[ivt][i] clusters..\n"); exit(1);}
            }
            for(int i = G->n_customers; i < (G->n_nodes); i++){
                K[ivt][i] = calloc(rows * n_size, sizeof *K[ivt][i]);
                if(!K[ivt][i]){perror("Error callocing K[ivt][i] clusters..\n"); exit(1);}
                createClusters(K[ivt][i], G, i, n_size, n_prim, n_sect, da_access[ivt]);
                //createClusters_drone(K[ivt][i], G, i, n_size, da_access[ivt]);
                counter++;
            }
        }else{
            for(int i = 0; i < (G->n_nodes); i++){
                K[ivt][i] = calloc(rows * n_size, sizeof *K[ivt][i]);
                if(!K[ivt][i]){perror("Error callocing K[ivt][i] clusters..\n"); exit(1);}
                createClusters(K[ivt][i], G, i, n_size, n_prim, n_sect, da_access[ivt]);
                counter++;
            }
        }
    }

    printf("Created %d/%d clusters.\n", counter, G->n_nodes);

    /*============================================*/

    /* Initialize asolution structs */

    asolution R, R_best;
    if(
        NULL == (R.a_VT = malloc(sizeof *R.a_VT * G->n_differentTypes)) ||
        NULL == (R_best.a_VT = malloc(sizeof *R_best.a_VT * G->n_differentTypes))
        
    )
    {
        perror("Error mallocing asolutions!\n");
        exit(1);
    }

    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        if(
            NULL == (R.a_VT[ivt].a_depots = malloc(sizeof *R.a_VT[ivt].a_depots * G->n_depots)) ||
            NULL == (R_best.a_VT[ivt].a_depots = malloc(sizeof *R_best.a_VT[ivt].a_depots * G->n_depots))
        )
        {
            perror("Error mallocing asolutions a_depots!\n");
            exit(1);
        }
        R.a_VT[ivt].IVT = ivt;
        R_best.a_VT[ivt].IVT = ivt;
        for(int idep = 0; idep < G->n_depots; idep++){
            R.a_VT[ivt].a_depots[idep].routelist = NULL;
            R.a_VT[ivt].a_depots[idep].depot_id = G->a_depots[idep].id;
            R.a_VT[ivt].a_depots[idep].makespan = 0.0;
            R.a_VT[ivt].a_depots[idep].quantity_served = 0;
            R.a_VT[ivt].a_depots[idep].current_load = -1;

            R_best.a_VT[ivt].a_depots[idep].routelist = NULL;
            R_best.a_VT[ivt].a_depots[idep].depot_id = G->a_depots[idep].id;
            R_best.a_VT[ivt].a_depots[idep].makespan = 0.0;
            R_best.a_VT[ivt].a_depots[idep].quantity_served = 0;
            R_best.a_VT[ivt].a_depots[idep].current_load = -1;
        }
    }

    //Probability matrix to calculate Shannon entropy
    int *edge_matrix;
    if(NULL == (edge_matrix = malloc(sizeof *edge_matrix * G->n_nodes*G->n_nodes))){
        perror("Error mallocing prob_matrix\n");
        exit(1);
    }

    R.total_makespan = HUGE_VAL;             //Initialize final solution distance
    int iter = 0, edge_sum = 0, stop_cond = 0, best_iter = -1;
    double term_condition = 1.0, foundtime = 0.0;
    time_t begin = time(NULL), loop = time(NULL), loop_time = difftime(loop, begin);
    /********** Main loop **********/
    while(stop_cond < 10000 && iter < 2000000 && term_condition > (0.01 - epsilon) && loop_time < 3600)
    {
        R_best.total_makespan = HUGE_VAL;
        int edge_sum = 0;
        memset(edge_matrix, 0, (G->n_nodes)*(G->n_nodes) * sizeof *edge_matrix);

        #pragma omp parallel for reduction(+:edge_sum)
        for(int i = 0; i < n_ants; i++){
            asolution Ra_local;
            if(NULL == (Ra_local.a_VT = malloc(sizeof *Ra_local.a_VT * G->n_differentTypes)))
            {
                perror("Error mallocing asolutions!\n");
                exit(1);
            }
            for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
                if(NULL == (Ra_local.a_VT[ivt].a_depots = malloc(sizeof *Ra_local.a_VT[ivt].a_depots * G->n_depots)))
                {
                    perror("Error mallocing asolutions a_depots!\n");
                    exit(1);
                }
                Ra_local.a_VT[ivt].IVT = ivt;
                for(int idep = 0; idep < G->n_depots; idep++){
                    Ra_local.a_VT[ivt].a_depots[idep].routelist = NULL;
                    Ra_local.a_VT[ivt].a_depots[idep].depot_id = G->a_depots[idep].id;
                }
            }
            antSolution(G, VT, K, phMatrix, &Ra_local, da_access, n_size, n_prim, a, b);
            
            edge_sum += store_edge_count(&Ra_local, G, edge_matrix);

            #pragma omp critical
            {
                if(Ra_local.total_makespan < R_best.total_makespan - epsilon){
                    new_best(&R_best, &Ra_local, G);
                }
            }
            // Free memory allocated for Ra_local within each iteration
            for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
                for(int idep = 0; idep < G->n_depots; idep++){
                    deleteList(&Ra_local.a_VT[ivt].a_depots[idep].routelist);
                }
                free(Ra_local.a_VT[ivt].a_depots);
            }
            free(Ra_local.a_VT);
        }

        for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
            printf("Type %d: %0.3lf\n", ivt+1, R_best.a_VT[ivt].makespan);
        }
        printf("R_best = %0.3lf\n", R_best.total_makespan);
        printf("R = %0.3lf\n", R.total_makespan);

        if (iter % n_freq == 0) {
            R_best.total_makespan = local_opt_full(&R_best, G, da_access, VT);
            printf("After local OPT R_best = %0.3lf\n", R_best.total_makespan);
        }


        if(R_best.total_makespan < R.total_makespan - epsilon){    
            time_t best = time(NULL);
            foundtime = difftime(best, begin);
            best_iter = iter;
            printf("\niter: %d\n%lf < %lf\n\n", iter, R_best.total_makespan, R.total_makespan);
            new_best(&R, &R_best, G);                       //R becomes the same as R_best
            stop_cond = 0;                                      //Reset stop_condition
        }

        if(T_update > 0.1)
            T_update -= a_update;
        printf("T_update %0.3lf\n", T_update);

        update_pheromones(G, phMatrix, &R, &R_best, T_update, d);  //Update Pheromone Matrices

        term_condition = evaporate_pheromones(G, edge_matrix, phMatrix, edge_sum, n_ants, p_min, p_max);
        printf("term_condition = %0.3lf\n", term_condition);

        iter++;
        stop_cond++;
        time_t loop = time(NULL);
        loop_time = difftime(loop, begin);
    }
    
    fprint_results(&R, G, VT);
    
    printf("\nTotal Makespan: %lf", R.total_makespan);

    printf("\n\nEnd of program...\n");
    printf("\nTerm_condition: %lf\n", term_condition);

    /* Free memory */
    free(phMatrix);
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int i = 0; i < G->n_nodes; i++) 
            free(K[ivt][i]);
        free(K[ivt]);
    }
    free(K);

    time_t end = time(NULL);
    
    printf("iter = %d\n", iter);

    double runtime = difftime(end, begin);
    printf("Total time = %lf\n", runtime);

    fprint_data(iter, best_iter, foundtime, runtime);

    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int idep = 0; idep < G->n_depots; idep++){
            deleteList(&R_best.a_VT[ivt].a_depots[idep].routelist);
            deleteList(&R.a_VT[ivt].a_depots[idep].routelist);
        }
        free(R_best.a_VT[ivt].a_depots);
        free(R.a_VT[ivt].a_depots);
    }

    free(edge_matrix);
    free(R_best.a_VT);
    free(R.a_VT);

    return;
}
