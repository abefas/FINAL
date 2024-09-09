#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/roulette.h"

int selectDepot(vt_solution *Ra, SON *G, int *v_free, int **K, double *phMatrix, int n_size, int n_prim){

    if(!Ra){ perror("Ra == NULL at selectDepot.c\n"); exit(1); }
    if(!G){ perror("G == NULL at selectDepot.c\n"); exit(1); }
    if(!v_free){ perror("v_free == NULL at selectDepot.c\n"); exit(1); }
    if(!K){ perror("K == NULL at selectDepot.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectDepot.c\n"); exit(1); }

    double prob_D, p_sum = 0.0;
    rlist *rl = NULL;
    int idepot, ilast, err, depot_id;
    node *v_cand = NULL;
    bool flag = true;
    //Get available depots for this vehicle type
    int N = n_prim;
    while(flag){
        for(idepot = 0; idepot < G->n_depots; idepot++){
            if(Ra->a_depots[idepot].v_d != -1){
                prob_D = 0.0;

                ilast = Ra->a_depots[idepot].v_d - 1;
                if(errCheck(ilast, 0, G->n_nodes-1) == 1){
                    perror("ilast error in selectDepot.c\n");
                    exit(1);
                }

                for(int icluster = 0; icluster < N; icluster++)
                    find_free_in_clusterk_VT(&v_cand, v_free, K[ilast], icluster, n_size);

                if(v_cand){
                    prob_D = calculate_pheromone_sum(ilast, v_cand, phMatrix, idepot, G->n_nodes);
                    deleteList(&v_cand);
                    if(prob_D > 0.0){
                        flag = false;
                        push_rlist(&rl, idepot, prob_D);
                        p_sum += prob_D;
                    }
                }

            }
        }
        if(flag)
            N++;    //free customer was not found in n_prim, search next cluster
    }

    if(p_sum == 0.0){
        idepot = -1;
    }else{
        idepot = roulette_list(rl, p_sum);
    }

    delete_rlist(&rl);

    return idepot;
}
