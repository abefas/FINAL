#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header_files/heuristic_main_functions.h"
#include "header_files/heuristic_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"


void heuristic_v1(SON *G, VType *VT, int **da_access){

    time_t begin = time(NULL);

    //Generate clusters - takes into consideration depots' vehicle availability
    //e.g. if closest depot has only drones but customer cannot be accessed by drones then customer 
    //goes into the second closest depot's cluster
    ClusterData cd = createClusters(G, da_access);

    /* Generate files to visualize the clustering *
    char fn[15];
    for(int dep = 0; dep < G->n_depots; dep++){
        printf("depot %d:\n", dep);
        sprintf(fn, "dep_%d.csv", dep);
        FILE *fp = fopen(fn, "w");
        fprintf(fp, "NodeID,Xposition,Yposition\n");
        for(int l = 0; l < cd.limit[dep]; l++){
            printf(" %d", cd.cluster[dep][l]);
            fprintf(fp, "%d,%d,%d\n", cd.cluster[dep][l], G->a_combined[cd.cluster[dep][l]-1].x, G->a_combined[cd.cluster[dep][l]-1].y);
        }
        if(fclose(fp) != 0){ printf("Error closing fp!\n"); exit(1); }
        printf("\n");
    }
    */


    asolution R;
    R.total_makespan = 0.0;
    if(NULL == (R.a_VT = malloc(sizeof *R.a_VT * G->n_differentTypes))){
        printf("Error mallocing R.a_VT in heuristic_prox.c\n");
        exit(1);
    }
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        if(NULL == (R.a_VT[ivt].a_depots = malloc(sizeof *R.a_VT[ivt].a_depots * G->n_depots))){
            printf("Error mallocing R.a_VT[%d].a_depots", ivt);
            exit(1);
        }
        R.a_VT[ivt].makespan = 0.0;
        R.a_VT[ivt].IVT = ivt;
        for(int idep = 0; idep < G->n_depots; idep++){
            R.a_VT[ivt].a_depots[idep].routelist = NULL;
            R.a_VT[ivt].a_depots[idep].quantity_served = 0;
            R.a_VT[ivt].a_depots[idep].depot_id = G->a_depots[idep].id;
            if(G->a_depots[idep].n_VT[ivt] == 0){
                R.a_VT[ivt].a_depots[idep].makespan = -1;
            }else{
                R.a_VT[ivt].a_depots[idep].makespan = 0.0;
            }
        }
    }

    adj_node **adj_matrix;
    if(NULL == (adj_matrix = malloc(sizeof **adj_matrix * G->n_nodes))){
        printf("Error mallocing adj_matrix!\n");
        exit(1);
    }
    for(int i = 0; i < G->n_nodes; i++)
        adj_matrix[i] = NULL;

    //Run heuristic for each depot and its clustered customers
    for(int IDEPOT = 0; IDEPOT < G->n_depots; IDEPOT++){

        //Adjacency matrix with customers in current cluster
        for(int i = 0; i < cd.limit[IDEPOT]; i++){
            adj_matrix[cd.cluster[IDEPOT][i] - 1] = create_adj_list(cd.cluster[IDEPOT][i], G, cd.cluster[IDEPOT], cd.limit[IDEPOT]);
        }
        //Depot adjacency to customers
        adj_matrix[G->a_depots[IDEPOT].id - 1] = create_adj_list(G->a_depots[IDEPOT].id, G, cd.cluster[IDEPOT], cd.limit[IDEPOT]);


        double distance = NearestNeighbour(adj_matrix, G, &R, da_access[0], cd.cluster[IDEPOT], cd.limit[IDEPOT], G->a_depots[IDEPOT].id, VT[0].capacity);
        if(!R.a_VT[0].a_depots[IDEPOT].routelist){
            printf("Depot doesn't have a truck!\n");
            exit(1);
        }
        R.a_VT[0].a_depots[IDEPOT].makespan = 
            get_makespan_depot_VT(G, R.a_VT[0].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);

        //Assign leftover customers to drone and motorbike
        initialization1(G, &R, VT, da_access, adj_matrix, cd.cluster[IDEPOT], cd.limit[IDEPOT], G->a_depots[IDEPOT].id);
        //We have makespan of each type for IDEPOT at this point so just run mfcmTSP heuristic here

    /***** MAIN HEURISTIC FUNCTION BEGINS HERE *****/
        int stop = 0, type, capacity;
        while(  G->a_depots[IDEPOT].n_VT[0] != 0    //IDEPOT has Truck(s)
                &&
                (
                    (
                        G->a_depots[IDEPOT].n_VT[1] != 0 &&
                        R.a_VT[1].a_depots[IDEPOT].makespan < R.a_VT[0].a_depots[IDEPOT].makespan - epsilon
                    ) 
                    || 
                    (   
                        G->a_depots[IDEPOT].n_VT[2] != 0 &&
                        R.a_VT[2].a_depots[IDEPOT].makespan < R.a_VT[0].a_depots[IDEPOT].makespan - epsilon
                    )
                ) 
                && stop == 0
        ){
            double diff_M, diff_D;
            if(G->a_depots[IDEPOT].n_VT[1] != 0){
                diff_M = R.a_VT[0].a_depots[IDEPOT].makespan - R.a_VT[1].a_depots[IDEPOT].makespan;
            }else{
                diff_M = -1;
            }
            
            if(G->a_depots[IDEPOT].n_VT[2] != 0){
                diff_D = R.a_VT[0].a_depots[IDEPOT].makespan - R.a_VT[2].a_depots[IDEPOT].makespan;
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

            double min_ms = R.a_VT[0].a_depots[IDEPOT].makespan;
            node *p = NULL, *s = R.a_VT[0].a_depots[IDEPOT].routelist->next;
            node *route_best = NULL;
            int index_nn = 0;
            int nn_length = listLength(R.a_VT[0].a_depots[IDEPOT].routelist);
            //While s points to a customer
            //Find the minimum cost route that can be added to selected type from Truck route
            while(index_nn < nn_length - 1){
                if(s->data > G->n_customers || da_access[type][s->data - 1] != 1){   //s not able to switch
                    s = s->next;
                    index_nn++;
                    continue;
                }
                p = s;
                int load = 0;
                node *successive_nodes = NULL;
                while(  p && 
                        p->data <= G->n_customers &&
                        load + G->a_customers[p->data - 1].demand <= capacity && 
                        da_access[type][p->data - 1] == 1)
                {
                    load += G->a_customers[p->data - 1].demand;
                    push(&successive_nodes, p->data);
                    p = p->next;
                }
                node *route = NULL;
                if(successive_nodes && listLength(successive_nodes) == VT[type].capacity){
                    route = copyList(successive_nodes);
                    push(&route, G->a_depots[IDEPOT].id);
                    append(&route, G->a_depots[IDEPOT].id);

                    node *l1 = copyList(R.a_VT[type].a_depots[IDEPOT].routelist);
                    node *l2 = copyList(route);

                    linkToTail(&l1, l2);

                    double dep_ms = 
                        get_makespan_depot_VT(G, l1, G->a_depots[IDEPOT].n_VT[type], VT[type].speed);
                    deleteList(&l1);
                    l2 = NULL;

                    //Check if swap has effect on Truck route
                    //Example p09.MDmfcmTSP where node 23 has no effect on IDEPOT 0 Truck route (standalone local opt on each swap)
                    l1 = copyList(R.a_VT[0].a_depots[IDEPOT].routelist);
                    l2 = successive_nodes;
                    while(l2){
                        deleteOnKey(&l1, l2->data);
                        l2 = l2->next;
                    }
                    double new_truck_ms = 
                        get_makespan_depot_VT(G, l1, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);
                    deleteList(&l1);
                    l2 = NULL;
                    if(new_truck_ms < 0.0 || dep_ms < 0.0){
                        printf("new_truck_ms || dep_ms < 0.0\n");
                        exit(1);
                    }
                    double max_ms;
                    if(dep_ms < new_truck_ms - epsilon){
                        max_ms = new_truck_ms;
                    }else{
                        max_ms = dep_ms;
                    }

                    if(max_ms < min_ms - epsilon){
                        min_ms = max_ms;
                        deleteList(&route_best);
                        route_best = copyList(route);
                    }

                    deleteList(&route);
                }
                deleteList(&successive_nodes);
                s = s->next;
                index_nn++;
            }

            if(min_ms < R.a_VT[0].a_depots[IDEPOT].makespan - epsilon){

                //Remove customers from Truck route
                p = route_best->next;
                while(p->next){
                    deleteOnKey(&R.a_VT[0].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }
                //Add customers to selected type route
                p = route_best;
                while(p){
                    push(&R.a_VT[type].a_depots[IDEPOT].routelist, p->data);
                    p = p->next;
                }

                R.a_VT[type].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R.a_VT[type].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[type], VT[type].speed);

                R.a_VT[0].a_depots[IDEPOT].makespan = 
                    get_makespan_depot_VT(G, R.a_VT[0].a_depots[IDEPOT].routelist, G->a_depots[IDEPOT].n_VT[0], VT[0].speed);

                remove_duplicate_nodes(&R.a_VT[0].a_depots[IDEPOT].routelist);
                remove_duplicate_nodes(&R.a_VT[type].a_depots[IDEPOT].routelist);
                double ms1 = k_optimization2(&R.a_VT[0].a_depots[IDEPOT], G, VT[0], 1);
                double ms2 = k_optimization2(&R.a_VT[0].a_depots[IDEPOT], G, VT[0], 2);
                if(type != 2){
                    ms1 = k_optimization2(&R.a_VT[type].a_depots[IDEPOT], G, VT[type], 1);
                    ms2 = k_optimization2(&R.a_VT[type].a_depots[IDEPOT], G, VT[type], 2);
                }
            }else{
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



    //Remove unnecessary nodes(depot to depot) and ready for final local opt
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int idep = 0; idep < G->n_depots; idep++){
           remove_duplicate_nodes(&R.a_VT[ivt].a_depots[idep].routelist);
        }
    }


    //Get Vehicle types' makespans - were not needed/used until now 
    //And resulting total makespan
    R.total_makespan = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        R.a_VT[ivt].makespan = get_makespan_VT(G, &R.a_VT[ivt]);

        printf("vt %d makespan: %0.2lf\n", ivt, R.a_VT[ivt].makespan);
        if(R.total_makespan < R.a_VT[ivt].makespan)
            R.total_makespan = R.a_VT[ivt].makespan;
    }

    R.total_makespan = local_opt_full(&R, G, da_access, VT);

    printf("total makespan = %0.2lf\n", R.total_makespan);

    time_t finish = time(NULL);
    double runtime = difftime(finish, begin);

    fprint_results(&R, G, VT);
    fprint_data(runtime);


    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int idep = 0; idep < G->n_depots; idep++){
            deleteList(&R.a_VT[ivt].a_depots[idep].routelist);
        }
        free(R.a_VT[ivt].a_depots);
    }
    free(R.a_VT);


    for(int dep = 0; dep < G->n_depots; dep++){
        free(cd.cluster[dep]);
    }
    free(cd.cluster);
    free(cd.limit);


    return;
}
