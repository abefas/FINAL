#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/roulette.h"
extern int loop_iteration;
/* Get pheromone info from prim clusters for Depots and for vehicle types  */

int selectVehicleType(int idepot, asolution *Ra, VType *VT, int *v_free, SON *G, int ***K, double *phMatrix, 
                      int **da_access, int n_size, int n_prim){

    if(!Ra){ perror("VT == NULL at selectVehicleType.c\n"); exit(1); }
    if(!G){ perror("G == NULL at selectVehicleType.c\n"); exit(1); }
    if(!v_free){ perror("v_free == NULL at selectVehicleType.c\n"); exit(1); }
    if(!K){ perror("K == NULL at selectVehicleType.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectVehicleType.c\n"); exit(1); }

    node *v_cand = NULL;
    rlist *rl = NULL;
    double p_sum = 0.0;
    //While no type has a free customer in its primary clusters, search next cluster(s)
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        double probVT = 0.0;
        if(G->a_depots[idepot].n_VT[ivt] != 0 && not_empty_clusters(v_free, G->n_customers, da_access[ivt])){
            int ilast = Ra->a_VT[ivt].a_depots[idepot].v_d - 1;
            if(errCheck(ilast, 0, G->n_nodes-1) == 1){
                perror("ilast error in selectVehicleType.c\n");
                exit(1);
            }
            for(int i_cluster = 0; i_cluster < n_prim; i_cluster++)
                find_free_in_clusterk_VT(&v_cand, v_free, K[ivt][ilast], i_cluster, n_size);
            if(v_cand){
                probVT = calculate_pheromone_sum(idepot, ivt, ilast, v_cand, phMatrix, G->n_nodes, G->n_differentTypes);
                deleteList(&v_cand);
                if(probVT > 0.0){
                    push_rlist(&rl, ivt, probVT);
                    p_sum += probVT;
                }
            }
        }
    }

    //If there are no available customers in the primary clusters
    //Search in the next clusters
    int s = n_prim, N = n_prim+1;
    while(p_sum == 0.0){
        for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
            double probVT = 0.0;
            if(G->a_depots[idepot].n_VT[ivt] != 0 && not_empty_clusters(v_free, G->n_customers, da_access[ivt])){
                int ilast = Ra->a_VT[ivt].a_depots[idepot].v_d - 1;
                if(errCheck(ilast, 0, G->n_nodes-1) == 1){
                    perror("ilast error in selectVehicleType.c\n");
                    exit(1);
                }
                for(int i_cluster = s; i_cluster < N; i_cluster++)
                    find_free_in_clusterk_VT(&v_cand, v_free, K[ivt][ilast], i_cluster, n_size);
                if(v_cand){
                    probVT = calculate_pheromone_sum(idepot, ivt, ilast, v_cand, phMatrix, G->n_nodes, G->n_differentTypes);
                    deleteList(&v_cand);
                    if(probVT > 0.0){
                        push_rlist(&rl, ivt, probVT);
                        p_sum += probVT;
                    }
                }
            }
        }
        N++;
        s++;
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
