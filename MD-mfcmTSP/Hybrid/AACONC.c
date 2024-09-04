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

extern int instance_id, AACORUN;

void AACONC(SON *G, VType *VT, int **da_access, int n_ants, 
            int n_freq, int n_size, int n_sect, int n_prim, double T_update, double a_update, 
            double p_min, double p_max, double d, double a, double b)
{

/* Initialize pheromone array */
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

    /* Clusters for each node (Customer and Depot) */
    int **K = malloc(sizeof *K * (G->n_nodes));
    int rows = (int)ceil((double)G->n_customers/n_size);
    int counter = 0;
    for(int i_node = 0; i_node < G->n_nodes; i_node++){
        if(NULL == (K[i_node] = calloc(rows * n_size, sizeof *K[i_node]))){
            printf("Error callocing K[i_node]\n");
            exit(1);
        }
        createClusters(K[i_node], G, i_node, n_size, n_sect);
        counter++;
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

    /* Initialize Final solution structs */
    asolution R, R_best;
    R.total_makespan = HUGE_VAL;
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
            if(ivt != 0)
                push(&R_best.a_VT[ivt].a_depots[idep].routelist, G->a_depots[idep].id);
        }
    }

    /* Initialize Truck structs */

    vt_solution Rt, Rt_best;

    if(
        NULL == (Rt.a_depots = malloc(sizeof *Rt.a_depots * G->n_depots)) ||
        NULL == (Rt_best.a_depots = malloc(sizeof *Rt_best.a_depots * G->n_depots))
    )
    {
        perror("Error mallocing asolutions a_depots!\n");
        exit(1);
    }

    Rt_best.IVT = Rt.IVT = 0;
    Rt.makespan = HUGE_VAL;
    //R_best.makespan is initialized in every iteration

    //route initialization
    for(int idep = 0; idep < G->n_depots; idep++){
        Rt.a_depots[idep].routelist = NULL;
        Rt.a_depots[idep].depot_id = G->a_depots[idep].id;
        Rt.a_depots[idep].makespan = 0.0;
        Rt.a_depots[idep].quantity_served = 0;
        Rt.a_depots[idep].current_load = -1;

        Rt_best.a_depots[idep].routelist = NULL;
        Rt_best.a_depots[idep].depot_id = G->a_depots[idep].id;
        Rt_best.a_depots[idep].makespan = 0.0;
        Rt_best.a_depots[idep].quantity_served = 0;
        Rt_best.a_depots[idep].current_load = -1;
    }

    //Probability matrix to calculate Shannon entropy
    int *edge_matrix;

    if(NULL == (edge_matrix = malloc(sizeof *edge_matrix * (G->n_nodes)*(G->n_nodes)))){
        printf("Error mallocing edge_matrix[]\n");
        exit(1);
    }

    int iter = 0, stop_cond = 0, best_iter = -1, edge_sum;
    double term_condition = 1.0, foundtime = 0.0, loop_time = 0.0;
    time_t begin = time(NULL);

    /********** Main loop **********/
    while(stop_cond < 5000 && iter < 1000000 && term_condition > (0.01 - epsilon) && loop_time < 3600)
    {
        Rt_best.makespan = HUGE_VAL;
        R_best.total_makespan = HUGE_VAL;
        edge_sum = 0;
        memset(edge_matrix, 0, (G->n_nodes)*(G->n_nodes) * sizeof *edge_matrix);

        #pragma omp parallel for reduction(+:edge_sum)
        for(int i = 0; i < n_ants; i++){
            vt_solution Ra_local;
            if(NULL == (Ra_local.a_depots = malloc(sizeof *Ra_local.a_depots * G->n_depots)))
            {
                perror("Error mallocing asolutions a_depots!\n");
                exit(1);
            }
            Ra_local.IVT = 0;
            for(int idep = 0; idep < G->n_depots; idep++){
                Ra_local.a_depots[idep].routelist = NULL;
                Ra_local.a_depots[idep].depot_id = G->a_depots[idep].id;
            }
            antSolution(G, VT[0], &Ra_local, K, phMatrix, n_size, n_prim, a, b);
            
            edge_sum += store_edge_count(&Ra_local, G, edge_matrix);

            #pragma omp critical
            {
                if(Ra_local.makespan < Rt_best.makespan - epsilon){
                    new_best_vt(&Rt_best, &Ra_local, G);
                }
            }
            for(int idep = 0; idep < G->n_depots; idep++){
                deleteList(&Ra_local.a_depots[idep].routelist);
            }
            free(Ra_local.a_depots);
        }

        //printf("\nRt_best = %0.3lf\n", Rt_best.makespan);
        //printf("Rt = %0.3lf\n", Rt.makespan);

        if (iter % n_freq == 0) {
            double og;
            bool flag = true;
            while(flag){
                for (int idep = 0; idep < G->n_depots; idep++) {
                    if(G->a_depots[idep].n_VT[0] != 0){
                        double ms1 = k_optimization2(&Rt_best.a_depots[idep], G, VT[0], 1);
                        double ms2 = k_optimization2(&Rt_best.a_depots[idep], G, VT[0], 2);
                    }
                }
                Rt_best.makespan = get_makespan_VT(G, &Rt_best);
                og = Rt_best.makespan;
                double ms1 = mutual_k_optimization(&Rt_best, G, VT[0], 2, 1);
                double ms2 = mutual_k_optimization(&Rt_best, G, VT[0], 2, 2);
                if(Rt_best.makespan >= og)
                    flag = false;
            }
        }


        //Copy Rt_best to R_best
        //Copy Truck routes to asolution routes
        for(int i = 0; i < G->n_depots; i++){
            R_best.a_VT[0].a_depots[i].routelist = copyList(Rt_best.a_depots[i].routelist);
        }
        
        //Offload customers to motorbikes 
        offload_to_motorbikes(G, VT, &R_best, &Rt_best, da_access, VT[1].capacity);

        //Get R_best makespan for Truck and Motorbikes
        for(int i = 0; i < G->n_depots; i++){
            R_best.a_VT[0].a_depots[i].makespan = get_makespan_depot_VT(G, R_best.a_VT[0].a_depots[i].routelist, G->a_depots[i].n_VT[0], 0);
            R_best.a_VT[1].a_depots[i].makespan = get_makespan_depot_VT(G, R_best.a_VT[1].a_depots[i].routelist, G->a_depots[i].n_VT[1], 1);
        }
        R_best.a_VT[0].makespan = get_makespan_VT(G, &R_best.a_VT[0]);
        R_best.a_VT[1].makespan = get_makespan_VT(G, &R_best.a_VT[1]);
        R_best.a_VT[2].makespan = 0;


        //Run heuristic to balance makespan to motorbikes and drones
        heuristic_v1_2(G, VT, &R_best, da_access);

        //printf("R_best = %0.2lf\n", R_best.total_makespan);
        //printf("R = %0.2lf\n", R.total_makespan);
    
        if(R_best.total_makespan < R.total_makespan - epsilon){    
            foundtime = difftime(time(NULL), begin);
            best_iter = iter;
            printf("\niter: %d\n%lf < %lf\n\n", iter, R_best.total_makespan, R.total_makespan);
            new_best(&R, &R_best, G);                       //R becomes the same as R_best
            new_best_vt(&Rt, &Rt_best, G);                  //Rt becomes the same as Rt_best
            stop_cond = 0;                                  //Reset stop_condition
        }


        //Update pheromones based on the final makespan (R_best,R)
        update_pheromone(G, &Rt, &Rt_best, &R, &R_best, phMatrix, T_update, d);  //Update Pheromone Matrices

        //Clear R_best
        for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
            R_best.a_VT[ivt].makespan = 0.0;
            for(int idep = 0; idep < G->n_depots; idep++){
                R_best.a_VT[ivt].a_depots[idep].makespan = 0.0;
                deleteList(&R_best.a_VT[ivt].a_depots[idep].routelist);
                push(&R_best.a_VT[ivt].a_depots[idep].routelist, G->a_depots[idep].id);
            }
        }

        term_condition = evaporate_pheromones(G, phMatrix, edge_matrix, edge_sum, n_ants, p_min, p_max);
        //printf("term_condition = %0.3lf\n", term_condition);

        iter++;
        stop_cond++;
        loop_time = difftime(time(NULL), begin);
    }
    
    double runtime = difftime(time(NULL), begin);

    fprint_results_VT(&Rt, G, VT, da_access[0]);
    //printf("\nRt Makespan: %lf", Rt.makespan);

    fprint_results(&R, G, VT, da_access);


    FILE *file;
    if((file = fopen("RESULTS_ALL.txt", "a")) == NULL){
        printf("Error appending result to file!\n");
        exit(1);
    }
    fprintf(file, "Instance %02d SD%d ND%d\nTotal Makespan = %0.2lf\nTime = %0.2lf s\n", 
            instance_id, (int)VT[2].speed, G->a_depots[0].n_VT[2], R.total_makespan, runtime);
    fclose(file);


    //printf("\n\nEnd of program...\n");
    //printf("\nTerm_condition: %lf\n", term_condition);

    /* Free memory */
    free(phMatrix);
    for(int i = 0; i < G->n_nodes; i++)
        free(K[i]);
    free(K);

    fprint_data_hybrid(VT, iter, best_iter, foundtime, runtime, term_condition, G->a_depots[0].n_VT[2]);

    for(int idep = 0; idep < G->n_depots; idep++){
        deleteList(&Rt_best.a_depots[idep].routelist);
        deleteList(&Rt.a_depots[idep].routelist);
    }
    free(Rt_best.a_depots);
    free(Rt.a_depots);

    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int idep = 0; idep < G->n_depots; idep++){
            deleteList(&R_best.a_VT[ivt].a_depots[idep].routelist);
            deleteList(&R.a_VT[ivt].a_depots[idep].routelist);
        }
        free(R_best.a_VT[ivt].a_depots);
        free(R.a_VT[ivt].a_depots);
    }
    free(R_best.a_VT);
    free(R.a_VT);


    free(edge_matrix);

    return;
}
