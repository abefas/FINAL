#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/roulette.h"

int selectDepot(asolution *Ra, int *v_free, SON *G, int **K, double *phMatrix, int *da_access, 
                int ivt, int n_size, int n_prim){

    if(!Ra){ perror("VT == NULL at selectDepot.c\n"); exit(1); }
    if(!v_free){ perror("v_free == NULL at selectDepot.c\n"); exit(1); }
    if(!G){ perror("G == NULL at selectDepot.c\n"); exit(1); }
    if(!*K){ perror("K == NULL at selectDepot.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectDepot.c\n"); exit(1); }

    double p_sum = 0.0;
    rlist *rl = NULL;
    int idepot, ilast, err, depot_id;
    node *v_cand = NULL;
    //Get available depots for this vehicle type
    for(idepot = 0; idepot < G->n_depots; idepot++){
        double prob_D = 0.0;
        if(G->a_depots[idepot].n_VT[ivt] != 0 && not_empty_clusters(v_free, G->n_customers, da_access)){
            ilast = Ra->a_VT[ivt].a_depots[idepot].v_d - 1;
            if(errCheck(ilast, 0, G->n_nodes-1) == 1){
                perror("ilast error in selectDepot.c\n");
                exit(1);
            }
            for(int icluster = 0; icluster < n_prim; icluster++)
                find_free_in_clusterk_VT(&v_cand, v_free, K[ilast], icluster, n_size);

            if(v_cand){
                prob_D = calculate_pheromone_sum(idepot, ivt, ilast, v_cand, phMatrix, G->n_nodes, G->n_differentTypes);
                deleteList(&v_cand);
                if(prob_D > 0.0){
                    push_rlist(&rl, idepot, prob_D);
                    p_sum += prob_D;
                }
            }
        }
    }

    if(p_sum == 0.0){
        printf("idepot == -1!\n");
        idepot = -1;
    }else{
        idepot = roulette_list(rl, p_sum);
    }

    delete_rlist(&rl);

    return idepot;
}
