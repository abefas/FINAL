#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header_files/structs.h"
#include "header_files/listFunctions.h"
#include "header_files/roulette.h"

int selectCustomer(int idepot, int ivt, int ilast, node *v_cand, double *phMatrix, SON *G, 
                   int *da_access, double a, double b){
    
    if(!v_cand){ perror("v_cand == NULL at selectCustomer.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectCustomer.c\n"); exit(1); }

    double p_sum = 0.0, prob, pheromones;
    rlist *rl = NULL;
    node *temp = v_cand;

    while(temp){
        pheromones = phMatrix[idepot*G->n_differentTypes*G->n_nodes*G->n_nodes 
                              + ivt*G->n_nodes*G->n_nodes 
                              + ilast*G->n_nodes 
                              + (temp->data - 1)];

        if(pheromones < 0.0 - epsilon){
            perror("pheromones < 0.0 in selectCustomer\n");
            exit(1);
        }

        prob = pow(G->d_matrix[ilast][temp->data - 1], -a) * pow(pheromones, b);
        if(prob > 0.0){
            push_rlist(&rl, temp->data - 1, prob);
            p_sum += prob;
        }
        temp = temp->next;
    }

    if(p_sum == 0){
        return v_cand->data-1;
    }

    int customer_index = roulette_list(rl, p_sum);

    delete_rlist(&rl);

    return customer_index;
}
