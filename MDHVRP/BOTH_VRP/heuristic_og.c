#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"

extern time_t begin_total;

void heuristic_og(SON *G, VType *VT, asolution *R, int **da_access, int *remaining){

    //Generate clusters - takes into consideration depots' vehicle availability
    //e.g. if closest depot has only drones but customer cannot be accessed by drones then customer 
    //goes into the second closest depot's cluster
    ClusterData cd = createClusters_prox(G, da_access, remaining);

    //Initialization1 here for Motorbike and Drones
    adj_node **adj_matrix;
    if(NULL == (adj_matrix = malloc(sizeof **adj_matrix * G->n_nodes))){
        printf("Error mallocing adj_matrix!\n");
        exit(1);
    }
    for(int i = 0; i < G->n_nodes; i++){
        adj_matrix[i] = NULL;
    }

    // heuristic for each depot and its clustered customers
    for(int IDEPOT = 0; IDEPOT < G->n_depots; IDEPOT++){

        //Adjacency matrix with customers in current cluster
        for(int i = 0; i < cd.limit[IDEPOT]; i++){
            adj_matrix[cd.cluster[IDEPOT][i] - 1] = create_adj_list(cd.cluster[IDEPOT][i], G, cd.cluster[IDEPOT], cd.limit[IDEPOT]);
        }
        //Depot adjacency to customers
        adj_matrix[G->a_depots[IDEPOT].id - 1] = create_adj_list(G->a_depots[IDEPOT].id, G, cd.cluster[IDEPOT], cd.limit[IDEPOT]);

        //Assign leftover customers to drone and motorbike
        initialization1(G, R, VT, da_access, adj_matrix, cd.cluster[IDEPOT], cd.limit[IDEPOT], G->a_depots[IDEPOT].id);

        for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
            R->a_VT[ivt].a_depots[IDEPOT].makespan = 
                get_makespan_depot_VT(G, R->a_VT[ivt].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[ivt], VT[ivt].speed);
        }

    /***** MAIN HEURISTIC FUNCTION BEGINS HER-> *****/
        int stop = 0, type, capacity;
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
                if(s->data > G->n_customers || G->a_customers[s->data - 1].demand > VT[type].capacity){   //s unavailable to switch
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
                        load + G->a_customers[(p->data - 1)].demand <= capacity && 
                        da_access[type][p->data - 1] == 1
                ){
                    load += G->a_customers[(p->data - 1)].demand;
                    push(&successive_nodes, p->data);
                    p = p->next;
                }

                node *route = NULL;
                if(successive_nodes){
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

                    if(dep_ms < min_ms - epsilon){
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

                //Delete nodes from Truck
                p = route_best->next;
                while(p->next){
                    deleteOnKey(&R->a_VT[0].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }

                //Add nodes to type
                p = route_best;
                while(p){
                    push(&R->a_VT[type].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }

                //Get new makespans for Truck and type
                R->a_VT[type].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R->a_VT[type].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[type], VT[type].speed);

                R->a_VT[0].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R->a_VT[0].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);

                //Run local opt on affected routes
                remove_duplicate_nodes(&R->a_VT[0].a_depots[IDEPOT].routelist);
                remove_duplicate_nodes(&R->a_VT[type].a_depots[IDEPOT].routelist);
                double ms1 = k_optimization2(&R->a_VT[0].a_depots[IDEPOT], G, VT[0], 1);
                double ms2 = k_optimization2(&R->a_VT[0].a_depots[IDEPOT], G, VT[0], 2);
                if(type != 2){
                    ms1 = k_optimization2(&R->a_VT[type].a_depots[IDEPOT], G, VT[type], 1);
                    ms2 = k_optimization2(&R->a_VT[type].a_depots[IDEPOT], G, VT[type], 2);
                }
                //Try depot_VT optimization here
            }else{
                //No swap possible
                printf("mt %0.2lf\t drone %0.2lf\n", R->a_VT[1].a_depots[IDEPOT].makespan, R->a_VT[2].a_depots[IDEPOT].makespan);
                printf("truck %0.2lf\n", R->a_VT[0].a_depots[IDEPOT].makespan);
                printf("min_cost %0.2lf\n", min_ms);
                printf("STOPPED\n");
                stop = 1;
            }
            deleteList(&route_best);    //best route was added to motorcycle or drone
        }
        //Continue to next depot
        deleteAdjLists(adj_matrix, G->n_nodes);
    }
    free(adj_matrix);



    //Get Vehicle types' makespans - were not needed/used until now 
    //And resulting total makespan
    R->total_makespan = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        R->a_VT[ivt].makespan = get_makespan_VT(G, &R->a_VT[ivt]);

        if(R->total_makespan < R->a_VT[ivt].makespan)
            R->total_makespan = R->a_VT[ivt].makespan;
    }

    R->total_makespan = depot_VT_optimization(R, G, VT, da_access, 1);
    R->total_makespan = depot_VT_optimization(R, G, VT, da_access, 2);
    R->total_makespan = local_opt_full(R, G, da_access, VT);
    R->total_makespan = depot_VT_optimization(R, G, VT, da_access, 1);
    R->total_makespan = depot_VT_optimization(R, G, VT, da_access, 2);

    time_t finish_total = time(NULL);
    double runtime_total = difftime(finish_total, begin_total);

    fprint_results(R, G, VT);
    fprint_data_total(runtime_total);

    for(int dep = 0; dep < G->n_depots; dep++){
        free(cd.cluster[dep]);
    }
    free(cd.cluster);
    free(cd.limit);

    return;
}

