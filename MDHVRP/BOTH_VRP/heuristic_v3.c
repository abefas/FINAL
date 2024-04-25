#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"

extern time_t begin_total;

//Run v3 for Truck routes of depots in which M_Truck is still bigger than any other M_type
void heuristic_v3(SON *G, VType *VT, asolution *R, int **da_access){

    for(int IDEPOT = 0; IDEPOT < G->n_depots; IDEPOT++){

        //Only enter if M_Truck is dep_ms
        if(R->a_VT[1].a_depots[IDEPOT].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon && 
            R->a_VT[2].a_depots[IDEPOT].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon){

            //Test successive nodes on IDEPOT's routes against Motorbikes and Drones of idep
            for(int idep = 0; idep < G->n_depots; idep++){
                //Skip if same depot OR 
                //depot has only Truck(s) OR 
                if( idep == IDEPOT || 
                    (G->a_depots[idep].n_VT[1] == 0 && G->a_depots[idep].n_VT[2] == 0)
                ) 
                    continue;

                //Else run heuristic using idep's vehicles on IDEPOT's route(s)
                int stop = 0;
                while(
                    (
                        R->a_VT[1].a_depots[idep].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon || 
                        R->a_VT[2].a_depots[idep].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon
                    ) && 
                    (
                        R->a_VT[1].a_depots[idep].makespan < R->a_VT[0].a_depots[idep].makespan - epsilon || 
                        R->a_VT[2].a_depots[idep].makespan < R->a_VT[0].a_depots[idep].makespan - epsilon
                    ) 
                    &&
                    stop == 0
                ){

                    int type, capacity, swap_type = -1;
                    double min_ms = R->a_VT[0].a_depots[IDEPOT].makespan;
                    node *route_best = NULL;
                    for(int t = 1; t < G->n_differentTypes; t++){
                        if(G->a_depots[idep].n_VT[t] == 0) continue;
                        type = t;
                        capacity = VT[type].capacity;
                        node *p = NULL, *s = R->a_VT[0].a_depots[IDEPOT].routelist->next;
                        int index_nn = 0;
                        int nn_length = listLength(R->a_VT[0].a_depots[IDEPOT].routelist);
                        //Find the optimal route that can be swapped to selected type from Truck route
                        while(index_nn < nn_length - 1){
                            bool flag = false;
                            if(s->data > G->n_customers){   //s points to depot
                                s = s->next;
                                index_nn++;
                                continue;
                            }
                            p = s;
                            int load = 0;
                            node *successive_nodes = NULL;
                            //Get at least one node
                            while(successive_nodes == NULL && p){
                                //Get as many successive nodes as possible to offload from the Truck
                                while(  p && p->data <= G->n_customers &&
                                        load + G->a_customers[p->data - 1].demand <= capacity && 
                                        da_access[type][p->data - 1] == 1
                                ){
                                    load += G->a_customers[p->data-1].demand;
                                    push(&successive_nodes, p->data);
                                    p = p->next;
                                }
                                p = p->next;
                            }
                            node *route = NULL;
                            if(successive_nodes && listLength(successive_nodes) == capacity){
                                printList(successive_nodes);
                                printf("\n");
                                route = copyList(successive_nodes);
                                push(&route, G->a_depots[idep].id);
                                append(&route, G->a_depots[idep].id);

                                node *l1 = copyList(R->a_VT[type].a_depots[idep].routelist);
                                node *l2 = copyList(route);

                                linkToTail(&l1, l2);
                                double dep_ms = 
                                    get_makespan_depot_VT(G, l1, G->a_depots[idep].n_VT[type], VT[type].speed);
                                deleteList(&l1);
                                l2 = NULL;

                                l1 = copyList(R->a_VT[0].a_depots[IDEPOT].routelist);
                                //Remove successive_nodes from Truck route and get new MS
                                l2 = successive_nodes;
                                while(l2){
                                    deleteOnKey(&l1, l2->data);
                                    l2 = l2->next;
                                }
                                //TEST THIS WITHOUT IDEPOT_NEW_MS
                                double IDEPOT_NEW_MS =  get_makespan_depot_VT(G, l1, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);
                                printf("dep_ms = %0.3lf\nIDEPOT_NEW_MS = %0.3lf\n", dep_ms, IDEPOT_NEW_MS);
                                deleteList(&l1);
                                l2 = NULL;

                                if(IDEPOT_NEW_MS < 0.0 || dep_ms < 0.0){
                                    printf("IDEPOT_NEW_MS || dep_ms < 0.0\n");
                                    exit(1);
                                }
                                double max_ms;
                                if(dep_ms < IDEPOT_NEW_MS - epsilon){
                                    max_ms = IDEPOT_NEW_MS;
                                }else{
                                    max_ms = dep_ms;
                                }
                                //We want the minimum possible max_ms
                                if(max_ms < min_ms - epsilon){    //min_ms is initialized as IDEPOT's Truck MS
                                    min_ms = max_ms;
                                    swap_type = type;
                                    deleteList(&route_best);
                                    route_best = copyList(route);
                                }
                                //else min_ms remains IDEPOT's Truck MS
                                deleteList(&route);
                            }
                            deleteList(&successive_nodes);
                            if(capacity > 1){
                                capacity--;
                            }else{
                                capacity = VT[type].capacity;
                                s = s->next;
                                index_nn++;
                            }
                        }
                    }
                    if(swap_type == -1 && min_ms < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon){
                        printf("min_ms = %0.3lf\nIDEPOT ms = %0.3lf\n", min_ms, R->a_VT[0].a_depots[IDEPOT].makespan);
                        exit(1);
                    }

                    //Do swap only if this condition holds
                    if(swap_type != -1 && min_ms < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon){

                        node *p = route_best->next;
                        while(p && p->next){
                            deleteOnKey(&R->a_VT[0].a_depots[IDEPOT].routelist, p->data);
                            p = p->next;
                        }
                        p = route_best;
                        while(p){
                            push(&R->a_VT[swap_type].a_depots[idep].routelist, p->data);
                            p = p->next;
                        }

                        R->a_VT[swap_type].a_depots[idep].makespan = 
                            get_makespan_depot_VT(G, R->a_VT[swap_type].a_depots[idep].routelist, 
                                               G->a_depots[idep].n_VT[swap_type], VT[swap_type].speed);

                        R->a_VT[0].a_depots[IDEPOT].makespan = 
                            get_makespan_depot_VT(G, R->a_VT[0].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);

                        remove_duplicate_nodes(&R->a_VT[0].a_depots[IDEPOT].routelist);
                        remove_duplicate_nodes(&R->a_VT[swap_type].a_depots[idep].routelist);
                    }else{
                        //No swap is possible
                        printf("mt %0.2lf\t drone %0.2lf\n", R->a_VT[1].a_depots[idep].makespan, R->a_VT[2].a_depots[idep].makespan);
                        printf("truck %0.2lf\n", R->a_VT[0].a_depots[IDEPOT].makespan);
                        printf("min_ms %0.2lf\n", min_ms);
                        printf("STOPPED\n");
                        stop = 1;
                    }
                    //local_optimization(G, R, VT);

                    deleteList(&route_best);    //best route was added to motorcycle or drone
                }
            }
        }
    }



    //R->move unnecessary nodes(depot to depot) and ready for final local opt
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int idep = 0; idep < G->n_depots; idep++){
           remove_duplicate_nodes(&R->a_VT[ivt].a_depots[idep].routelist);
        }
    }


    //Get Vehicle types' makespans - were not needed/used until now 
    //And resulting total makespan
    R->total_makespan = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        R->a_VT[ivt].makespan = get_makespan_VT(G, &R->a_VT[ivt]);

        printf("vt %d makespan: %0.2lf\n", ivt, R->a_VT[ivt].makespan);
        if(R->total_makespan < R->a_VT[ivt].makespan)
            R->total_makespan = R->a_VT[ivt].makespan;
    }



    //Do final local opt (mutual)
    //local_optimization(G, R, VT);
    //When mutual local opt improves a vehicle type's makespan, run single local opt for that type's
    //depots again
    //While mutual local opt improves, run single opt again



    printf("total makespan = %0.2lf\n", R->total_makespan);

    time_t finish_total = time(NULL);
    double runtime_total = difftime(finish_total, begin_total);

    fprint_results_v3(R, G, VT);
    fprint_data_total(runtime_total);

    return;
}


