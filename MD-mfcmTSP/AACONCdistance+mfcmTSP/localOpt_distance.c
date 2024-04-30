#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/local_opt.h"

double local_opt_full_distance(d_solution *R, SON *G){

    bool flag = true;
    while(flag){
        for (int idep = 0; idep < G->n_depots; idep++) {
            double ms1 = k_optimization2_distance(&R->a_depots[idep], G, 1);
            double ms2 = k_optimization2_distance(&R->a_depots[idep], G, 2);
        }
        //R->total_distance = get_makespan_VT(G, &R);
        double og = R->total_distance;
        double ms1 = mutual_k_optimization_distance(R, G, 1, 1);
        double ms2 = mutual_k_optimization_distance(R, G, 1, 2);

        double ms3 = mutual_k_optimization_distance(R, G, 2, 1);
        double ms4 = mutual_k_optimization_distance(R, G, 2, 2);

        if(ms1 < og - epsilon || ms2 < og - epsilon || ms3 < og - epsilon || ms4 < og - epsilon){
            flag = true;
        }else{
            flag = false;
        }
    }

    return R->total_distance;
}


double k_optimization2_distance(droute *r, SON *G, int n_max){
    if( !r->routelist ||
        !r->routelist->next ||
        listLength(r->routelist) == 3
    ) return r->distance;

    int list_position = 1, node_position, list_length, i_node, idepot = r->depot_id - G->n_customers - 1;
    double ms_new, ms, ms_best, ms_orig;
    node *successive_nodes = NULL, *p1 = NULL;
    node *p = NULL, *bestroute = NULL;

    node *depotroute = copyList(r->routelist);
    ms = ms_best = ms_orig = r->distance;
    list_position = 1;
    list_length = listLength(depotroute);
    // move nodes in route 
    while(list_position < list_length-n_max){                           //For every node(s) in depotroute
        // Get successive_nodes
        for(int i = 0; i < n_max; i++){
            i_node = getNth(depotroute, list_position);             //Get node(s) value
            if(i_node > G->n_customers) {                           //Don't move depot node(s)
                break;
            }         
            deleteInPosition(&depotroute, list_position+1);             //Delete node(s) from current position
            append(&successive_nodes, i_node);                    //Add to a separate list
        }
        if(!successive_nodes || listLength(successive_nodes) != n_max){
            deleteList(&successive_nodes);
            deleteList(&depotroute);
            if(bestroute){
                depotroute = copyList(bestroute);
            }else{
                depotroute = copyList(r->routelist);
            }
            list_position++;
            continue;
        }
        // Move successive_nodes to every position in depotroute
        node_position = 1;
        p = depotroute;
        while(p && p->next){                    //While depotroute has nodes, place successive nodes after them
            node_position++;
            p1 = successive_nodes;
            while(p1){
                insertAfterNode(p, p1->data);
                p1 = p1->next;
                p = p->next;
            }
            //Check capacity feasibility, compare new makespan to old, update best if new route better
            int check = check_route_feasibility_distance(depotroute, G);
            if(check == 0){
                ms_new = get_route_distance(G, depotroute);
                if(ms_new < ms_best - epsilon){
                    deleteList(&bestroute);
                    bestroute = copyList(depotroute);
                    ms_best = ms_new;
                }
            }
            p = p->next;
            for(int i = 0; i < listLength(successive_nodes); i++)
                deleteInPosition(&depotroute, node_position);           //reset depotroute and continue moving successive_node(s)
        }
        //Prepare to move next successive node(s)
        deleteList(&successive_nodes);
        //Update depotroute with best found solution so far if found and run again on new route
        //else move to next node
        if(ms_best < ms - epsilon){
            remove_duplicate_nodes(&bestroute);                          // If a move to depot was eliminated then remove extra depot
            deleteList(&depotroute);
            depotroute = copyList(bestroute);
            ms = ms_best;
            list_length = listLength(depotroute);
        }else{
            deleteList(&depotroute);
            if(bestroute){
                depotroute = copyList(bestroute);
            }else{
                depotroute = copyList(r->routelist);
            }
            list_position++;
        }
    }
    //Update route with best solution (if found) 
    if(ms_best < ms_orig - epsilon){
        deleteList(&r->routelist);
        r->routelist = copyList(bestroute);
        r->distance = ms_best;
        //quantity_served remains the same in single colony optimization
    }

    deleteList(&depotroute);
    deleteList(&bestroute);

    remove_duplicate_nodes(&r->routelist);

    return r->distance;
}

double mutual_k_optimization_distance(d_solution *R, SON *G, int condition, int n_max){

    double ms_og = R->total_distance;

    node *successive_nodes = NULL, *l1 = NULL, *l2 = NULL, *p1 = NULL;

    for(int i = 0; i < G->n_depots; i++)
        push(&R->a_depots[i].routelist, R->a_depots[i].routelist->data);

    //Begin mutual colony optimization
    int idep = 0, sec;
    while(idep < G->n_depots){
        if(!R->a_depots[idep].routelist->next){idep++; continue;}
        sec = 0;
        while(sec < G->n_depots){
            if( idep == sec || 
                !R->a_depots[sec].routelist->next
            ){ sec++; continue; }
            l1 = copyList(R->a_depots[idep].routelist);
            int length_l1 = listLength(l1);
            int l1_position = 1, i_node;
            // move nodes in route
            while(l1_position < length_l1-n_max){                                   //For every node(s) in l1
                double idep_og = get_route_distance(G, l1);
                // Get successive_nodes 
                for(int i = 0; i < n_max; i++){
                    i_node = getNth(l1, l1_position);                               //Get node(s) value
                    if(i_node > G->n_customers) {                                   //Don't move depot node(s)
                        break;
                    }         
                    deleteInPosition(&l1, l1_position+1);
                    append(&successive_nodes, i_node);
                }
                if(!successive_nodes || listLength(successive_nodes) != n_max){
                    deleteList(&successive_nodes);
                    deleteList(&l1);
                    l1 = copyList(R->a_depots[idep].routelist);
                    l1_position++;
                    continue;
                }

                double idep_new = get_route_distance(G, l1);
                bool flag = false;

                deleteList(&l2);
                l2 = copyList(R->a_depots[sec].routelist);
                double sec_og = get_route_distance(G, l2);
                // Move successive_nodes to every position in depotroute 
                int node_position = 1;
                node *p = l2;
                while(p && p->next){
                    node_position++;
                    p1 = successive_nodes;
                    while(p1){
                        insertAfterNode(p, p1->data);
                        p1 = p1->next;
                        p = p->next;
                    }
                    //Check capacity feasibility
                    int check = check_route_feasibility_distance(l2, G);
                    if(check == 0){

                        double sec_new = get_route_distance(G, l2);

                        if(condition == 1){
                            if(idep_new < idep_og && sec_new < R->a_depots[sec].distance)
                            {
                                deleteList(&R->a_depots[idep].routelist);
                                deleteList(&R->a_depots[sec].routelist);
                                R->a_depots[idep].routelist = copyList(l1);
                                R->a_depots[sec].routelist = copyList(l2);
                                R->a_depots[idep].distance = idep_new;
                                R->a_depots[sec].distance = sec_new;
                                flag = true;
                            }
                        }else if(condition == 2){
                            if(idep_new + sec_new < idep_og + sec_og)
                            {
                                deleteList(&R->a_depots[idep].routelist);
                                deleteList(&R->a_depots[sec].routelist);
                                R->a_depots[idep].routelist = copyList(l1);
                                R->a_depots[sec].routelist = copyList(l2);
                                R->a_depots[idep].distance = idep_new;
                                R->a_depots[sec].distance = sec_new;
                                flag = true;
                            }
                        }else{
                            printf("Unknown condition in mutual opt\n");
                            exit(1);
                        }
                    }
                    p = p->next;
                    for(int i = 0; i < listLength(successive_nodes); i++)
                        deleteInPosition(&l2, node_position);
                }
                if(flag){                                                           //Swapped node_data to depot sec
                    //If swap was made, get the updated l1 route
                    deleteList(&l1);
                    l1 = copyList(R->a_depots[idep].routelist);
                    length_l1 = listLength(l1);
                    //node(s) where removed so index_l1 increment is not needed
                }else{
                    l1_position++;
                    deleteList(&l1);
                    l1 = copyList(R->a_depots[idep].routelist);
                }
                deleteList(&l2);
                deleteList(&successive_nodes);
            }
            //Changing sec
            deleteList(&l1);
            sec++;
        }
        //Changing idep
        idep++;
    }

    for(int i = 0; i < G->n_depots; i++)
        remove_duplicate_nodes(&R->a_depots[i].routelist);

    R->total_distance = 0.0;
    for(int idep = 0; idep < G->n_depots; idep++){
        R->total_distance += R->a_depots[idep].distance;
    }

    return R->total_distance;
}

int check_route_feasibility_distance(node *route, SON *G){
    node *p = route;
    int limit = 0;
    while(p){
        if(p->data <= G->n_customers && p->data > 0){
            limit += G->a_customers[p->data-1].demand;
            if(limit > G->capacity)
                return 1;               //Infeasible
        }else if(p->data > G->n_customers && p->data <= G->n_nodes){
            limit = 0;
        }else{
            printf("Invalid nodes in route in check_route_feasibility\n");
            printf("node %d\n", p->data);
            exit(EXIT_SUCCESS);
        }
        p = p->next;
    }
    return 0;                           //Feasible
}

double get_route_distance(SON *G, node *list){
    if(!list) return 0;
    double distance = 0.0;
    node *temp = list;
    while(temp && temp->next){
        distance += G->d_matrix[temp->data - 1][temp->next->data - 1];
        temp = temp->next;
    }
    return distance;
}
