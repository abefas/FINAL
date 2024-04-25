#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"

void antSolution(SON *G, VType *VT, int ***K, double *phMatrix, asolution *Ra, int **da_access, 
                int n_size, int n_prim, double a, double b){

    /* Initialize v_free array which contains not yet visited Customer vertices */
    int v_free[G->n_customers];
    for(int i = 0; i < G->n_customers; i++)
        v_free[i] = 1; //free = 1, not free = -1

    int ivt, idepot, ilast, icluster, icustomer, launch_count[G->n_differentTypes], served_count[G->n_differentTypes];

    for(int i = 0; i < G->n_differentTypes; i++)
        launch_count[i] = served_count[i] = 0;

    node *v_candidates = NULL;

    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        Ra->a_VT[ivt].makespan = 0.0;
        for(int idep = 0; idep < G->n_depots; idep++){
            if(G->a_depots[idep].n_VT[ivt] != 0){
                deleteList(&Ra->a_VT[ivt].a_depots[idep].routelist);
                Ra->a_VT[ivt].a_depots[idep].makespan = 0.0;
                Ra->a_VT[ivt].a_depots[idep].quantity_served = 0;
                Ra->a_VT[ivt].a_depots[idep].v_d = Ra->a_VT[ivt].a_depots[idep].depot_id;
                Ra->a_VT[ivt].a_depots[idep].current_load = VT[ivt].capacity;
                push(&Ra->a_VT[ivt].a_depots[idep].routelist, Ra->a_VT[ivt].a_depots[idep].depot_id);
            }else{
                Ra->a_VT[ivt].a_depots[idep].v_d = -1;
                Ra->a_VT[ivt].a_depots[idep].current_load = -1;
            }
        }
    }

    /*********** Main loop ***********/

    while(not_empty(v_free, G->n_customers))
    { 
        ivt = (idepot = (icluster = (icustomer = -1)));

        while(ivt == -1 || idepot == -1 || icluster == -1 || icustomer == -1){

            idepot = selectDepot(Ra, v_free, G, K, phMatrix, da_access, n_size, n_prim);
            if(idepot == -1)
                continue;


            ivt = selectVehicleType(idepot, Ra, VT, v_free, G, K, phMatrix, launch_count, served_count, da_access, n_size, n_prim);
            if(ivt == -1)
                continue;
            if(ivt < 0 || ivt > G->n_differentTypes - 1){
                printf("ivt out of bounds!\n");
                exit(1);
            }


            ilast = Ra->a_VT[ivt].a_depots[idepot].v_d - 1;
            if(errCheck(ilast, 0, G->n_nodes-1) == 1){ 
                perror("ilast error in antSolution.c\n"); 
                exit(1); 
            }

            icluster = selectCluster(idepot, ivt, ilast, v_free, K[ivt][ilast], phMatrix, G, 
                                     n_size, n_prim, VT[ivt].speed, a, b);
            if(icluster == -1)
                continue;

            find_free_in_clusterk_VT(&v_candidates, v_free, K[ivt][ilast], icluster, n_size); 
            if(!v_candidates){
                perror("Could not find free in cluster k in function antSolution\n");
                exit(1);
            }

            icustomer = selectCustomer(idepot, ivt, ilast, v_candidates, phMatrix, G, da_access[2], a, b);

            deleteList(&v_candidates);
        }

        if(Ra->a_VT[ivt].a_depots[idepot].v_d == Ra->a_VT[ivt].a_depots[idepot].depot_id){
            launch_count[ivt]++;
        }

        /* Insert selected customer to route and update values makespan, ilast, vehicle's load and v_free */
        append(&Ra->a_VT[ivt].a_depots[idepot].routelist, icustomer+1);
        served_count[ivt]++;

        //Drone immediately returns to depot after serving one customer, thus location is never changed also
        if(ivt == 2){
            append(&Ra->a_VT[ivt].a_depots[idepot].routelist, Ra->a_VT[ivt].a_depots[idepot].depot_id);
            Ra->a_VT[ivt].a_depots[idepot].quantity_served += G->a_customers[icustomer].demand;
        }else{
            Ra->a_VT[ivt].a_depots[idepot].v_d = icustomer + 1;
            Ra->a_VT[ivt].a_depots[idepot].current_load -= G->a_customers[icustomer].demand;
            if(Ra->a_VT[ivt].a_depots[idepot].current_load == 0){
                append(&Ra->a_VT[ivt].a_depots[idepot].routelist, Ra->a_VT[ivt].a_depots[idepot].depot_id);
                Ra->a_VT[ivt].a_depots[idepot].current_load = VT[ivt].capacity;
                Ra->a_VT[ivt].a_depots[idepot].v_d = Ra->a_VT[ivt].a_depots[idepot].depot_id;
            }
            Ra->a_VT[ivt].a_depots[idepot].quantity_served += G->a_customers[icustomer].demand;
        }

        v_free[icustomer] = -1;      //Mark as visited
    }

    /* Vehicles return to depot */
    int last, depotID;
    for(int idep = 0; idep < G->n_depots; idep++){
        for(ivt = 0; ivt < G->n_differentTypes; ivt++){
            if(G->a_depots[idep].n_VT[ivt] != 0){
                if(Ra->a_VT[ivt].a_depots[idep].v_d != Ra->a_VT[ivt].a_depots[idep].depot_id){
                    append(&Ra->a_VT[ivt].a_depots[idep].routelist, Ra->a_VT[ivt].a_depots[idep].depot_id);
                }
                Ra->a_VT[ivt].a_depots[idep].makespan = 
                    get_makespan_depot_VT(G, Ra->a_VT[ivt].a_depots[idep].routelist, G->a_depots[idep].n_VT[ivt], VT[ivt].speed);
            }
        }
    }

    //Get makespan of solution
    Ra->total_makespan = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        Ra->a_VT[ivt].makespan = get_makespan_VT(G, &Ra->a_VT[ivt]);
        if(Ra->a_VT[ivt].makespan > Ra->total_makespan)
            Ra->total_makespan = Ra->a_VT[ivt].makespan;
    }

    return;
}

