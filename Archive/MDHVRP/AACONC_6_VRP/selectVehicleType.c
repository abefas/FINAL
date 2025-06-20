#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/roulette.h"

/* Get pheromone info from prim clusters for Depots and for vehicle types  */

int selectVehicleType(asolution *Ra, VType *VT, int *v_free, SON *G, int ***K, double *phMatrix, 
                      int *launch_count, int **da_access, int n_size, int n_prim){

    if(!Ra){ perror("VT == NULL at selectVehicleType.c\n"); exit(1); }
    if(!G){ perror("G == NULL at selectVehicleType.c\n"); exit(1); }
    if(!v_free){ perror("v_free == NULL at selectVehicleType.c\n"); exit(1); }
    if(!K){ perror("K == NULL at selectVehicleType.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectVehicleType.c\n"); exit(1); }


    double prob_cap[3] = {0}, cap_sum = 0.0;
    for(int i = 0; i < G->n_differentTypes; i++){
        if(not_empty_clusters(v_free, G->n_customers, da_access[i])){
            prob_cap[i] = VT[i].n_vehicles * VT[i].capacity;
            cap_sum += prob_cap[i];
        }
    }
    for(int i = 0; i < G->n_differentTypes; i++){
        prob_cap[i] /= cap_sum;
    }

    double probVT[3] = {0};
    node *v_cand = NULL;

    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        if(not_empty_clusters(v_free, G->n_customers, da_access[ivt])){
            for(int idepot = 0; idepot < G->n_depots; idepot++){
                if(G->a_depots[idepot].n_VT[ivt] != 0){

                    int ilast = Ra->a_VT[ivt].a_depots[idepot].v_d - 1;
                    if(errCheck(ilast, 0, G->n_nodes-1) == 1){
                        perror("ilast error in selectDepot.c\n");
                        exit(1);
                    }

                    for(int i_cluster = 0; i_cluster < n_prim; i_cluster++)
                        find_free_in_clusterk_VT(&v_cand, v_free, K[ivt][ilast], i_cluster, n_size);

                    if(v_cand){
                        int numOfCand = listLength(v_cand);
                        probVT[ivt] += calculate_pheromone_sum(idepot, ivt, ilast, v_cand, phMatrix, G->n_nodes, G->n_differentTypes)/numOfCand;
                        deleteList(&v_cand);
                    }
                }
            }
        }
    }
    rlist *rl = NULL;
    double p_sum = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        double prob;
        if(probVT[ivt] > 0.0){
            if(launch_count[ivt] > 0){
                prob = probVT[ivt]/launch_count[ivt];
                //prob = pow(probVT[ivt]/launch_count[ivt], prob_cap[ivt]);
            }else{
                prob = pow(probVT[ivt], prob_cap[ivt]);
            }
            push_rlist(&rl, ivt, prob);
            p_sum += prob;
        }
    }

    if(p_sum == 0.0){
        perror("psum == 0 at selectVehicleType");
        for(int i = 0; i < G->n_customers; i++){
            if(v_free[i] == 1)
                printf("Free customer: %d\n", i+1);
        }
        exit(1);
    }else{
        int ivt = roulette_list(rl, p_sum);
        delete_rlist(&rl);
        return ivt;
    }
}
