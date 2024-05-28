#include <stdlib.h>
#include "header_files/heuristic_misc_functions.h"
#include "header_files/structs.h"

void initialization1(SON *G, asolution *R, VType *VT, int **da_access, 
                     adj_node **adj_matrix, int *cluster, int n, int depotID){

    int v_free[G->n_customers];
    for(int i = 0; i < G->n_customers; i++)
        v_free[i] = 0;

    //Get customers that are not in Truck route
    for(int i = 0; i < n; i++){
        if(da_access[0][cluster[i] - 1] == 0)    //Not truck accessible
            v_free[cluster[i] - 1] = 1;
    }

    //All customers are in Truck NN
    if(!not_empty(v_free, G->n_customers)){
        return;
    }

    int idep = depotID - G->n_customers - 1;
    if(G->a_depots[idep].n_VT[1] == 0 && G->a_depots[idep].n_VT[2] == 0 && not_empty(v_free, G->n_customers)){
        printf("init1 error: Not enough vehicles in depot!");
        exit(1);
    }

    //Initialization 1 - 
    //Epilegei ton kontinotero pelati kathe fora 
    //an epileksei Motorbike ftiaxnei route toso oso to capacity tou me nearest neighbour
    bool sw = false;
    while(not_empty(v_free, G->n_customers)){
        if((R->a_VT[2].a_depots[idep].makespan < R->a_VT[1].a_depots[idep].makespan - epsilon) && 
            (G->a_depots[idep].n_VT[2] != 0) && 
            sw == false)
        {
            adj_node *p = adj_matrix[depotID - 1];
            while(p && (v_free[p->id-1] == 0 || da_access[2][p->id - 1] == 0)){
                p = p->next;
            }
            if(!p){
                //printf("Couldn't find customer for Drone!\n");
                sw += true;
                continue;
            }
            push(&R->a_VT[2].a_depots[idep].routelist, depotID);
            push(&R->a_VT[2].a_depots[idep].routelist, p->id);
            push(&R->a_VT[2].a_depots[idep].routelist, depotID);
            R->a_VT[2].a_depots[idep].makespan = 
                get_makespan_depot_VT(G, R->a_VT[2].a_depots[idep].routelist, G->a_depots[idep].n_VT[2], VT[2].speed);
            v_free[p->id - 1] = 0;
        }else{
            node *route_best = NULL;
            int cap = VT[1].capacity, load = 0;
            push(&R->a_VT[1].a_depots[idep].routelist, depotID);
            adj_node *p = adj_matrix[depotID - 1];
            while(p && load < cap){
                while(p && (v_free[p->id - 1] == 0 || da_access[1][p->id - 1] == 0)){
                    p = p->next;
                }
                if(!p) break;

                push(&R->a_VT[1].a_depots[idep].routelist, p->id);
                v_free[p->id - 1] = 0;
                load++;
                p = adj_matrix[p->id - 1];

            }
            push(&R->a_VT[1].a_depots[idep].routelist, depotID);

            if(listLength(R->a_VT[1].a_depots[idep].routelist) == 2){
                printf("Error in init1 - no customer was added to Motorbike route\n");
                exit(1);
            }

            R->a_VT[1].a_depots[idep].makespan = 
                get_makespan_depot_VT(G, R->a_VT[1].a_depots[idep].routelist, G->a_depots[idep].n_VT[1], VT[1].speed);
        }
    }

    return;
}
