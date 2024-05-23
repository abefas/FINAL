#include <stdio.h>
#include <stdlib.h>
#include "header_files/heuristic_main_functions.h"
#include "header_files/heuristic_misc_functions.h"
#include "header_files/structs.h"

double NearestNeighbour(adj_node **adj_matrix, SON *G, asolution *R, int *da_access, int *cluster, int n, int depotID, int truck_capacity){
    double d_total = 0;

    int *v_free_T;
    if(NULL == (v_free_T = calloc(G->n_customers, sizeof *v_free_T))){
        perror("Error callocing v_free_T!\n");
        exit(1);
    }

    for(int i = 0; i < n; i++){
        if(da_access[cluster[i] - 1] == 1)
            v_free_T[cluster[i] - 1] = 1;
    }

    int idep = depotID - G->n_customers - 1, load = 0;
    push(&R->a_VT[0].a_depots[idep].routelist, depotID);
    adj_node *p = adj_matrix[depotID - 1];
    while(not_empty(v_free_T, G->n_customers)){
        while(p && v_free_T[p->id - 1] != 1){
            p = p->next;
        }
        if(!p){
            printf("!p in NearestNeighbour!\n");
            exit(1);
        }
        append(&R->a_VT[0].a_depots[idep].routelist, p->id);
        load += G->a_customers[p->id - 1].demand;
        d_total += p->distance;
        v_free_T[p->id - 1] = 0;
        if(load == truck_capacity){
            append(&R->a_VT[0].a_depots[idep].routelist, depotID);
            load = 0;
            p = adj_matrix[depotID - 1];
        }else{
            p = adj_matrix[p->id - 1];
        }
    }
    append(&R->a_VT[0].a_depots[idep].routelist, depotID);

    free(v_free_T);
    return d_total;
}
