#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"

extern time_t begin_total;

void heuristic_og_no_init(SON *G, VType *VT, asolution *R, int **da_access, int *remaining){


    // heuristic for each depot and its clustered customers
    for(int IDEPOT = 0; IDEPOT < G->n_depots; IDEPOT++){

    /***** MAIN HEURISTIC FUNCTION BEGINS HER-> *****/
        int stop = 0, type, capacity;
        while(  G->a_depots[IDEPOT].n_VT[0] != 0
                &&
                (
                    (   
                        G->a_depots[IDEPOT].n_VT[1] != 0 && 
                        R->a_VT[1].a_depots[IDEPOT].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon
                    )
                    || 
                    (   
                        G->a_depots[IDEPOT].n_VT[2] != 0 && 
                        R->a_VT[2].a_depots[IDEPOT].makespan < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon
                    )
                ) 
                && stop == 0
        ){
            double diff_M, diff_D;
            if(G->a_depots[IDEPOT].n_VT[1] != 0){
                diff_M = R->a_VT[0].a_depots[IDEPOT].makespan - R->a_VT[1].a_depots[IDEPOT].makespan;
            }else{
                diff_M = -1;
            }
            
            if(G->a_depots[IDEPOT].n_VT[2] != 0){
                diff_D = R->a_VT[0].a_depots[IDEPOT].makespan - R->a_VT[2].a_depots[IDEPOT].makespan;
            }else{
                diff_D = -1;
            }
            if(diff_M == -1 && diff_D == -1){
                printf("heuristic entered IDEPOT with only Trucks\n");
                exit(1);
            }

            if(diff_M >= diff_D){
                //choose motorcycle
                type = 1;
                capacity = VT[type].capacity;
            }else{
                //choose drone
                type = 2;
                capacity = VT[type].capacity;
            }

            double min_ms = HUGE_VAL;
            node *p = NULL, *s = R->a_VT[0].a_depots[IDEPOT].routelist->next;
            node *route_best = NULL;
            int index_nn = 0;
            int nn_length = listLength(R->a_VT[0].a_depots[IDEPOT].routelist);
            //Find the minimum cost route that can be swapped to selected type from Truck route
            while(index_nn < nn_length - 1){
                if(s->data > G->n_customers || da_access[type][s->data - 1] != 1){   //s points to depot
                    s = s->next;
                    index_nn++;
                    continue;
                }
                p = s;
                int load = 0;
                node *successive_nodes = NULL;
                //Get as many successive nodes as possible to offload from the Truck
                while(  p && 
                        p->data <= G->n_customers &&
                        load + G->a_customers[p->data - 1].demand <= capacity && 
                        da_access[type][p->data - 1] == 1
                ){
                    load += G->a_customers[p->data-1].demand;
                    push(&successive_nodes, p->data);
                    p = p->next;
                }
                node *route = NULL;
                if(successive_nodes && listLength(successive_nodes) == VT[type].capacity){
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

                    //Check if swap has effect on Truck route
                    //Example p09.MDmfcmTSP where node 23 has no effect on IDEPOT 0 Truck route 
                    //(proximity heuristic, standalone local opt on each swap)
                    l1 = copyList(R->a_VT[0].a_depots[IDEPOT].routelist);
                    l2 = successive_nodes;
                    while(l2){
                        deleteOnKey(&l1, l2->data);
                        l2 = l2->next;
                    }
                    double new_truck_ms = 
                        get_makespan_depot_VT(G, l1, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);
                    deleteList(&l1);
                    l2 = NULL;


                    if(dep_ms < min_ms - epsilon && new_truck_ms < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon){
                        min_ms = dep_ms;
                        deleteList(&route_best);
                        route_best = copyList(route);
                    }

                    deleteList(&route);
                }
                deleteList(&successive_nodes);
                s = s->next;
                index_nn++;
            }

            if(min_ms < R->a_VT[0].a_depots[IDEPOT].makespan - epsilon){

                p = route_best->next;
                while(p->next){
                    deleteOnKey(&R->a_VT[0].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }
                p = route_best;
                while(p){
                    push(&R->a_VT[type].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }
                R->a_VT[type].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R->a_VT[type].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[type], VT[type].speed);

                R->a_VT[0].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R->a_VT[0].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);

                remove_duplicate_nodes(&R->a_VT[0].a_depots[IDEPOT].routelist);
                remove_duplicate_nodes(&R->a_VT[type].a_depots[IDEPOT].routelist);
                double ms1 = k_optimization2(&R->a_VT[0].a_depots[IDEPOT], G, VT[0], 1);
                double ms2 = k_optimization2(&R->a_VT[0].a_depots[IDEPOT], G, VT[0], 2);
                if(type != 2){
                    ms1 = k_optimization2(&R->a_VT[type].a_depots[IDEPOT], G, VT[type], 1);
                    ms2 = k_optimization2(&R->a_VT[type].a_depots[IDEPOT], G, VT[type], 2);
                }
            }else{
                //No swap possible
                printf("min_cost %0.2lf\n", min_ms);
                printf("STOPPED\n");
                stop = 1;
            }
            deleteList(&route_best);    //best route was added to motorcycle or drone
        }
        //Continue to next depot
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

        if(R->total_makespan < R->a_VT[ivt].makespan)
            R->total_makespan = R->a_VT[ivt].makespan;
    }

    R->total_makespan = local_opt_full(R, G, da_access, VT);

    time_t finish_total = time(NULL);
    double runtime_total = difftime(finish_total, begin_total);

    fprint_results(R, G, VT, da_access);
    fprint_data_total(runtime_total);

    return;
}


