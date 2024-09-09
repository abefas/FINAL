#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/roulette.h"

int selectCluster(SON *G, int ilast, int *v_free, int *K, double *phMatrix, int idepot,  
                  int n_size, int n_prim, double vspeed, double a, double b){

    if(!v_free){ perror("v_free == NULL at selectCluster.c\n"); exit(1); }
    if(!K){ perror("K == NULL at selectCluster.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectCluster.c\n"); exit(1); }
    if(!G){ perror("G == NULL at selectCluster.c\n"); exit(1); }

    node *v_cand = NULL;
    rlist *rl = NULL;
    double h[n_prim], t[n_prim], p[n_prim], p_sum = 0.0, h_sum = 0.0, t_sum = 0.0;
    int icluster;

    for(icluster = 0; icluster < n_prim; icluster++){
        find_free_in_clusterk_VT(&v_cand, v_free, K, icluster, n_size);
        if(!v_cand){
            h[icluster] = (t[icluster] = 0.0);
        }else{
            double numOfCand = listLength(v_cand);
            h[icluster] = calculate_h(G, v_cand, ilast) * numOfCand;     
            t[icluster] = calculate_pheromone_sum(ilast, v_cand, phMatrix, idepot, G->n_nodes) / numOfCand;               
            deleteList(&v_cand);
            h_sum += pow(h[icluster],a);
            t_sum += pow(t[icluster],b);
        }
    }

    //Return first cluster with free customer(s)
    if(h_sum == 0.0){
        for(int icluster = n_prim; icluster < (int)ceil((double)G->n_customers / n_size); icluster++){
            if(search_cluster_VT(K, icluster, v_free, n_size)){
                return icluster;
            }
        }
    }else{
        double prob;
        for(icluster = 0; icluster < n_prim; icluster++){
            if(h[icluster] - epsilon > 0.0 && t[icluster] - epsilon > 0.0){
                prob = pow(h[icluster],a) * pow(t[icluster],b) / (h_sum*t_sum);
                push_rlist(&rl, icluster, prob);
                p_sum += prob;
            }
        }
        icluster = roulette_list(rl, p_sum);
    }

    delete_rlist(&rl);

    return icluster;
}
