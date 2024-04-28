#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"

void antSolution(SON *G, VType VT, vt_solution *Ra, int **K, double *phMatrix, int *da_access, int *remaining, 
                int n_size, int n_prim, double a, double b){

    /* Initialize v_free array which contains not yet visited Customer vertices */
    int v_free[G->n_customers];
    for(int i = 0; i < G->n_customers; i++){
        if(da_access[i] == 1 && remaining[i] == 1){
            v_free[i] = 1;      //free = 1, not free = -1
        }else{
            v_free[i] = -1;
        }
    }

    int idepot, ilast, icluster, icustomer;

    node *v_candidates = NULL;

    Ra->makespan = 0.0;
    for(int idep = 0; idep < G->n_depots; idep++){          //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        if(G->a_depots[idep].n_VT[Ra->IVT] != 0){           //ADD FAKE TRUCK TO DEPOTS WITH MOTORBIKES AND NO TRUCKS!!!!!!!!!!
            deleteList(&Ra->a_depots[idep].routelist);
            Ra->a_depots[idep].makespan = 0.0;
            Ra->a_depots[idep].quantity_served = 0;
            Ra->a_depots[idep].v_d = Ra->a_depots[idep].depot_id;
            Ra->a_depots[idep].current_load = VT.capacity;
            push(&Ra->a_depots[idep].routelist, Ra->a_depots[idep].depot_id);
        }else{
            Ra->a_depots[idep].v_d = -1;
            Ra->a_depots[idep].current_load = -1;
        }
    }

    /*********** Main loop ***********/

    while(not_empty(v_free, G->n_customers))
    { 
        idepot = (icluster = (icustomer = -1));

        while(idepot == -1 || icluster == -1 || icustomer == -1){

            idepot = selectDepot(Ra, G, v_free, K, phMatrix, n_size, n_prim);
            if(idepot == -1){
                continue;
            }

            ilast = Ra->a_depots[idepot].v_d - 1;
            if(errCheck(ilast, 0, G->n_nodes-1) == 1){ 
                perror("ilast error in antSolution.c\n"); 
                exit(1); 
            }

            icluster = selectCluster(G, ilast, v_free, K[ilast], phMatrix, idepot, n_size, n_prim, VT.speed, a, b);
            if(icluster == -1){
                continue;
            }

            find_free_in_clusterk_VT(&v_candidates, v_free, K[ilast], icluster, n_size); 
            if(!v_candidates){
                perror("Could not find free in cluster k in function antSolution\n");
                exit(1);
            }

            icustomer = selectCustomer(G, idepot, ilast, v_candidates, phMatrix, a, b);

            deleteList(&v_candidates);
        }

        //Return to depot to reload
        if(Ra->a_depots[idepot].current_load < G->a_customers[icustomer].demand){
            append(&Ra->a_depots[idepot].routelist, Ra->a_depots[idepot].depot_id);
            Ra->a_depots[idepot].current_load = VT.capacity;
        }

        /* Insert selected customer to route and update values */
        append(&Ra->a_depots[idepot].routelist, icustomer+1);
        Ra->a_depots[idepot].v_d = icustomer + 1;
        Ra->a_depots[idepot].current_load -= G->a_customers[icustomer].demand;
        Ra->a_depots[idepot].quantity_served += G->a_customers[icustomer].demand;

        v_free[icustomer] = -1;      //Mark customer as visited
    }

    /* Vehicles return to depot */
    for(int idep = 0; idep < G->n_depots; idep++){
        if(G->a_depots[idep].n_VT[Ra->IVT] != 0){
            if(Ra->a_depots[idep].v_d != Ra->a_depots[idep].depot_id){
                append(&Ra->a_depots[idep].routelist, Ra->a_depots[idep].depot_id);
            }
            Ra->a_depots[idep].makespan = 
                get_makespan_depot_VT(G, Ra->a_depots[idep].routelist, G->a_depots[idep].n_VT[Ra->IVT], VT.speed);
        }
    }

    //Get makespan of vt_solution
    Ra->makespan = get_makespan_VT(G, Ra);

    return;
}

