#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/roulette.h"

int selectCustomer(SON *G, int idepot, int ilast, node *v_cand, double *phMatrix, double a, double b){
    
    if(!v_cand){ perror("v_cand == NULL at selectCustomer.c\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at selectCustomer.c\n"); exit(1); }

    double p_sum = 0.0, prob, pheromones;
    rlist *rl = NULL;
    node *temp = v_cand;

    while(temp){
        pheromones = phMatrix[(idepot*G->n_nodes*G->n_nodes) + (ilast*G->n_nodes) + (temp->data - 1)];
        if(pheromones < 0.0){
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

    int customer_index = roulette_list(rl, p_sum);
    //Case where customer(s) in v_cand have pheromone trail == 0 - updated to not be possible
    if(customer_index == -1){ customer_index = v_cand->data - 1; }
    delete_rlist(&rl);
    return customer_index;
}
