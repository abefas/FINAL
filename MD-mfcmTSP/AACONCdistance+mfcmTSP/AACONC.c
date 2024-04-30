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

void AACONC(SON *G, VType *VT, vt_solution *Rz, int **da_access, int *remaining, int ivt, int n_ants, 
            int n_freq, int n_size, int n_sect, int n_prim, double T_update, double a_update, 
            double p_min, double p_max, double d, double a, double b)
{

    //omp_set_num_threads(200);
/* Initialize 4-D pheromone array */
    double *phMatrix = malloc(sizeof *phMatrix * G->n_depots*G->n_nodes*G->n_nodes);
    for(int d = 0; d < G->n_depots; d++){
        for(int k = 0; k < G->n_nodes; k++){
            for(int l = 0; l < G->n_nodes; l++){
                //node to itself OR depot to depot or Depot doesn't support vehicle type
                if(k == l || (k >= G->n_customers && l >= G->n_customers)){   
                    phMatrix[d*G->n_nodes*G->n_nodes + k*G->n_nodes + l] = -1.0;
                }else{
                    phMatrix[d*G->n_nodes*G->n_nodes + k*G->n_nodes + l] = 1.0;
                }
            }
        }
    }

    /* Clusters for each vehicle type and each node (Customer and Depot) */
    int **K = malloc(sizeof *K * (G->n_nodes));
    int rows = (int)ceil((double)G->n_customers/n_size);
    int counter = 0;
    for(int i_node = 0; i_node < G->n_nodes; i_node++){
        if(NULL == (K[i_node] = calloc(rows * n_size, sizeof *K[i_node]))){
            printf("Error callocing K[i_node]\n");
            exit(1);
        }
        if(da_access[ivt][i_node] == 1){
            createClusters(K[i_node], G, i_node, n_size, n_sect, da_access[ivt]);
            counter++;
        }
    }

    printf("Created %d/%d clusters.\n", counter, G->n_nodes);
    /*
    printf("Node 51 (Depot 1):\n");
    for(int i = 0; i < rows; i++){
        printf("\nRow %d:\n", i+1);
        for(int j = 0; j < n_size; j++)
            printf("%d ", K[50][i * n_size + j]);
    }
    exit(1);
    */

    /*============================================*/

    /* Initialize asolution structs */

    d_solution R, R_best;

    if(
        NULL == (R.a_depots = malloc(sizeof *R.a_depots * G->n_depots)) ||
        NULL == (R_best.a_depots = malloc(sizeof *R_best.a_depots * G->n_depots))
    )
    {
        perror("Error mallocing asolutions a_depots!\n");
        exit(1);
    }

    R.total_distance = HUGE_VAL;
    //route initialization
    for(int idep = 0; idep < G->n_depots; idep++){
        R.a_depots[idep].routelist = NULL;
        R.a_depots[idep].depot_id = G->a_depots[idep].id;
        R.a_depots[idep].distance = 0.0;
        R.a_depots[idep].quantity_served = 0;
        R.a_depots[idep].current_load = -1;

        R_best.a_depots[idep].routelist = NULL;
        R_best.a_depots[idep].depot_id = G->a_depots[idep].id;
        R_best.a_depots[idep].distance = 0.0;
        R_best.a_depots[idep].quantity_served = 0;
        R_best.a_depots[idep].current_load = -1;
    }

    //Probability matrix to calculate Shannon entropy
    int *edge_matrix;

    if(NULL == (edge_matrix = malloc(sizeof *edge_matrix * (G->n_nodes)*(G->n_nodes)))){
        printf("Error mallocing edge_matrix[]\n");
        exit(1);
    }

    int iter = 0, stop_cond = 0, best_iter = -1, edge_sum;
    double term_condition = 1.0, foundtime = 0.0;
    time_t begin = time(NULL), loop = time(NULL), loop_time = difftime(loop, begin);

    /********** Main loop **********/
    while(stop_cond < 4000 && iter < 100000 && term_condition > (0.1 - epsilon) && loop_time < 3600)
    {
        R_best.total_distance = HUGE_VAL;
        edge_sum = 0;
        memset(edge_matrix, 0, (G->n_nodes)*(G->n_nodes) * sizeof *edge_matrix);

        #pragma omp parallel for reduction(+:edge_sum)
        for(int i = 0; i < n_ants; i++){
            d_solution Ra_local;
            if(NULL == (Ra_local.a_depots = malloc(sizeof *Ra_local.a_depots * G->n_depots)))
            {
                perror("Error mallocing asolutions a_depots!\n");
                exit(1);
            }
            for(int idep = 0; idep < G->n_depots; idep++){
                Ra_local.a_depots[idep].routelist = NULL;
                Ra_local.a_depots[idep].depot_id = G->a_depots[idep].id;
            }
            antSolution(G, VT[ivt], &Ra_local, K, phMatrix, da_access[ivt], remaining, n_size, n_prim, a, b);
            
            edge_sum += store_edge_count(&Ra_local, G, edge_matrix);

            #pragma omp critical
            {
                if(Ra_local.total_distance < R_best.total_distance - epsilon){
                    new_best(&R_best, &Ra_local, G);
                }
            }
            for(int idep = 0; idep < G->n_depots; idep++){
                deleteList(&Ra_local.a_depots[idep].routelist);
            }
            free(Ra_local.a_depots);
        }

        printf("\nR_best = %0.3lf\n", R_best.total_distance);
        printf("R = %0.3lf\n", R.total_distance);

        if (iter % n_freq == 0) {
            R_best.total_distance = local_opt_full_distance(&R_best, G);
        }

        if(R_best.total_distance < R.total_distance - epsilon){    
            time_t best = time(NULL);
            foundtime = difftime(best, begin);
            best_iter = iter;
            printf("\niter: %d\n%lf < %lf\n\n", iter, R_best.total_distance, R.total_distance);
            new_best(&R, &R_best, G);                       //R becomes the same as R_best
            stop_cond = 0;                                  //Reset stop_condition
        }


        T_update *= a_update;

        update_pheromones(G, phMatrix, &R, &R_best, T_update, d);

        term_condition = evaporate_pheromones(G, phMatrix, edge_matrix, da_access[ivt], edge_sum, n_ants, p_min, p_max);
        printf("term_condition = %0.3lf\n", term_condition);

        iter++;
        stop_cond++;
        time_t loop = time(NULL);
        loop_time = difftime(loop, begin);
    }
    
    fprint_results_AACONC(&R, G, VT[0], da_access[ivt]);

    //Update remaining[]
    for(int idep = 0; idep < G->n_depots; idep++){
        node *p = R.a_depots[idep].routelist;
        while(p){
            if(p->data <= G->n_customers)
                remaining[p->data - 1] = -1;

            p = p->next;
        }
    }
    
    /* Copy Solution to Rz */
    for(int idep = 0; idep < G->n_depots; idep++){
        deleteList(&Rz->a_depots[idep].routelist);
        Rz->a_depots[idep].routelist = copyList(R.a_depots[idep].routelist);
        Rz->a_depots[idep].quantity_served = R.a_depots[idep].quantity_served;
        Rz->a_depots[idep].makespan = get_makespan_depot_VT(G, R.a_depots[idep].routelist, G->a_depots[idep].n_VT[0], VT[0].speed);
    }
    Rz->makespan = get_makespan_VT(G, Rz);

    /* Free memory */
    free(phMatrix);
    for(int i = 0; i < G->n_nodes; i++)
        free(K[i]);
    free(K);

    time_t end = time(NULL);
    
    printf("iter = %d\n", iter);

    double runtime = difftime(end, begin);
    printf("Total time = %lf\n", runtime);

    fprint_data_AACONC(ivt, iter, best_iter, foundtime, runtime);

    for(int idep = 0; idep < G->n_depots; idep++){
        deleteList(&R_best.a_depots[idep].routelist);
        deleteList(&R.a_depots[idep].routelist);
    }
    free(R_best.a_depots);
    free(R.a_depots);

    free(edge_matrix);

    return;
}
