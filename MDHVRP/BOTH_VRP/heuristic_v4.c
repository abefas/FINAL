#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"

extern time_t begin_total;

void heuristic_v4(SON *G, VType *VT, asolution *R, int **da_access){

    printf("Entered heuristic\n");

    // heuristic for each depot and its clustered customers
    for(int IDEPOT = 0; IDEPOT < G->n_depots; IDEPOT++){

        /***** MAIN HEURISTIC FUNCTION BEGINS HERE *****/
        int stop = 0;
        while(
            (
                (G->a_depots[IDEPOT].n_VT[1] != 0 && 
                R->a_VT[1].a_depots[IDEPOT].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon)
                || 
                (G->a_depots[IDEPOT].n_VT[2] != 0 && 
                R->a_VT[2].a_depots[IDEPOT].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon)
            )
            && 
            stop == 0
        ){

            int type, capacity, swap_type = -1;
            double min_ms = R->a_VT[0].a_depots[IDEPOT].makespan;
            node *route_best = NULL;
            for(int t = 1; t < G->n_differentTypes; t++){
                if(G->a_depots[IDEPOT].n_VT[t] == 0) continue;
                type = t;
                capacity = VT[type].capacity;


                node *p = NULL, *s = R->a_VT[0].a_depots[IDEPOT].routelist->next;
                int index_nn = 0;
                int nn_length = listLength(R->a_VT[0].a_depots[IDEPOT].routelist);
                //Find the minimum cost route that can be swapped to selected type from Truck route
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
                            da_access[type][p->data - 1] == 1)
                        {
                            load += G->a_customers[p->data-1].demand;
                            push(&successive_nodes, p->data);
                            p = p->next;
                        }
                        p = p->next;
                    }
                    node *route = NULL;
                    if(successive_nodes && listLength(successive_nodes) == capacity){
                        route = copyList(successive_nodes);
                        push(&route, G->a_depots[IDEPOT].id);
                        append(&route, G->a_depots[IDEPOT].id);

                        node *l1 = copyList(R->a_VT[type].a_depots[IDEPOT].routelist);
                        node *l2 = copyList(route);

                        linkToTail(&l1, l2);
                        double dep_ms = 
                            get_makespan_depot_VT(G, l1, G->a_depots[IDEPOT].n_VT[type], VT[type].speed);
                        deleteList(&l1);
                        l2 = NULL;

                        l1 = copyList(R->a_VT[0].a_depots[IDEPOT].routelist);
                        //Remove successive_nodes from Truck route and get updated MS
                        l2 = successive_nodes;
                        while(l2){
                            deleteOnKey(&l1, l2->data);
                            l2 = l2->next;
                        }
                        double IDEPOT_NEW_MS = get_makespan_depot_VT(G, l1, G->a_depots[IDEPOT].n_VT[0],  VT[0].speed);
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

                        if(max_ms < min_ms - epsilon){
                            min_ms = max_ms;
                            swap_type = type;
                            deleteList(&route_best);
                            route_best = copyList(route);
                        }
                        //else min_ms remains Truck's MS
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
            if(swap_type != -1 && min_ms < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon){

                node *p = route_best->next;
                while(p->next){
                    deleteOnKey(&R->a_VT[0].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }
                p = route_best;
                while(p){
                    push(&R->a_VT[swap_type].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }
                R->a_VT[swap_type].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R->a_VT[swap_type].a_depots[IDEPOT].routelist, 
                                        G->a_depots[IDEPOT].n_VT[swap_type], VT[swap_type].speed);

                R->a_VT[0].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R->a_VT[0].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);

                //Local opt here - not mutual
                remove_duplicate_nodes(&R->a_VT[0].a_depots[IDEPOT].routelist);
                remove_duplicate_nodes(&R->a_VT[swap_type].a_depots[IDEPOT].routelist);
                double ms1 = k_optimization2(&R->a_VT[0].a_depots[IDEPOT], G, VT[0], 1);
                double ms2 = k_optimization2(&R->a_VT[0].a_depots[IDEPOT], G, VT[0], 2);

                if(swap_type != 2){
                    ms1 = k_optimization2(&R->a_VT[swap_type].a_depots[IDEPOT], G, VT[swap_type], 1);
                    ms2 = k_optimization2(&R->a_VT[swap_type].a_depots[IDEPOT], G, VT[swap_type], 2);
                }

            }else{
                //no swap possible
                printf("mt %0.2lf\t drone %0.2lf\n", R->a_VT[1].a_depots[IDEPOT].makespan, R->a_VT[2].a_depots[IDEPOT].makespan);
                printf("truck %0.2lf\n", R->a_VT[0].a_depots[IDEPOT].makespan);
                printf("min_cost %0.2lf\n", min_ms);
                printf("STOPPED\n");
                stop = 1;
            }
            deleteList(&route_best);    //best route was added to motorcycle or drone
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

        if(R->a_VT[ivt].makespan > R->total_makespan)
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

    fprint_results(R, G, VT);
    fprint_data_total(runtime_total);

    return;
}
