#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"
#include "header_files/local_opt.h"
#include "header_files/AACO_misc_functions.h"

double local_opt_full(asolution *R, SON *G, int **da_access, VType *VT){

    //R->total_makespan = depot_VT_optimization(R, G, VT, da_access, 1);
    //R->total_makespan = depot_VT_optimization(R, G, VT, da_access, 2);

    for (int ivt = 0; ivt < G->n_differentTypes; ivt++) {
        bool flag = true;
        while(flag){
            if (ivt != 2) {
                for (int idep = 0; idep < G->n_depots; idep++) {
                    if(G->a_depots[idep].n_VT[ivt] != 0){
                        double ms1 = k_optimization2(&R->a_VT[ivt].a_depots[idep], G, VT[ivt], 1);
                        double ms2 = k_optimization2(&R->a_VT[ivt].a_depots[idep], G, VT[ivt], 2);
                    }
                }
                R->a_VT[ivt].makespan = get_makespan_VT(G, &R->a_VT[ivt]);
                double og = R->a_VT[ivt].makespan;
                double ms1 = mutual_k_optimization(&R->a_VT[ivt], G, VT[ivt], 2, 1);
                double ms2 = mutual_k_optimization(&R->a_VT[ivt], G, VT[ivt], 2, 2);
                if(ms1 < og - epsilon || ms2 < og - epsilon){
                    flag = true;
                }else{
                    flag = false;
                }
            } else {
                double og = R->a_VT[ivt].makespan;
                double ms = mutual_drone(&R->a_VT[ivt], G, VT[ivt]);
                if(ms < og - epsilon){
                    flag = true;
                }else{
                    flag = false;
                }
            }
        }
    }

    R->total_makespan = get_total_makespan(R,  G->n_differentTypes);
    //R->total_makespan = depot_VT_optimization(R,  G, VT, da_access, 1);
    //R->total_makespan = depot_VT_optimization(R,  G, VT, da_access, 2);

    return R->total_makespan;
}


double depot_VT_optimization(asolution *R, SON *G, VType *VT, int **da_access, int n_max){

    double ms_og = R->total_makespan;

    node *successive_nodes = NULL, *l1 = NULL, *l2 = NULL, *p1 = NULL;

    //For every depot with at least two different vehicle types, swap the customers between vehicles
    //Swap if new_max_ms < max_ms
    for(int idep = 0; idep < G->n_depots; idep++){
        for(int t = 0; t < G->n_differentTypes; t++){
            push(&R->a_VT[t].a_depots[idep].routelist, G->a_depots[idep].id);
        }
        //Removing duplicates at the end of idep loop

        for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
            if(G->a_depots[idep].n_VT[ivt] == 0 || !R->a_VT[ivt].a_depots[idep].routelist->next)
            {
                continue;
            }
            for(int ivt2 = 0; ivt2 < G->n_differentTypes; ivt2++){
                if(ivt == ivt2 || G->a_depots[idep].n_VT[ivt2] == 0 || !R->a_VT[ivt2].a_depots[idep].routelist->next)
                {
                    continue;
                }
                l1 = copyList(R->a_VT[ivt].a_depots[idep].routelist);
                int length_l1 = listLength(l1);
                int l1_position = 1, i_node;
                int n_max_t = n_max;
                if(ivt == 2 || ivt2 == 2) n_max_t = 1;
                // move nodes in route
                while(l1_position < length_l1-n_max_t){                                   //For every node(s) in l1
                    //Get ms of the two types
                    double max_ms;
                    if(R->a_VT[ivt].a_depots[idep].makespan < R->a_VT[ivt2].a_depots[idep].makespan - epsilon){
                        max_ms = R->a_VT[ivt2].a_depots[idep].makespan;
                    }else{
                        max_ms = R->a_VT[ivt].a_depots[idep].makespan;
                    }
                    //Get successive_nodes 
                    for(int i = 0; i < n_max_t; i++){
                        i_node = getNth(l1, l1_position);                               //Get node(s) value
                        if(i_node > G->n_customers || da_access[ivt2][i_node - 1] != 1){//Check node
                            break;
                        }         
                        deleteInPosition(&l1, l1_position+1);
                        append(&successive_nodes, i_node);
                    }
                    if(!successive_nodes || listLength(successive_nodes) != n_max_t){
                        deleteList(&successive_nodes);
                        deleteList(&l1);
                        l1 = copyList(R->a_VT[ivt].a_depots[idep].routelist);
                        l1_position++;
                        continue;
                    }

                    double ivt_ms = get_makespan_depot_VT(G, l1, G->a_depots[idep].n_VT[ivt], VT[ivt].speed);
                    bool flag = false;  //Signals if a swap has been made

                    deleteList(&l2);
                    l2 = copyList(R->a_VT[ivt2].a_depots[idep].routelist);
                    //Move successive_nodes to every position in the route of ivt2
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
                        int check = check_route_feasibility(l2, G, VT[ivt2].capacity);

                        if(check == 0){

                            double ivt2_ms = get_makespan_depot_VT(G, l2, G->a_depots[idep].n_VT[ivt2], VT[ivt2].speed);

                            double new_max_ms;
                            if(ivt_ms < ivt2_ms - epsilon){
                                new_max_ms = ivt2_ms;
                            }else{
                                new_max_ms = ivt_ms;
                            }
                            //Accept swap if
                            //new_max_ms < max_ms || (new_max_ms <= max_ms && sec_ms < R->a_depots[sec].makespan)
                            //if max_ms is idep_ms then max_ms cannot change while moving nodes in sec
                            //if max_ms is sec_ms then swap is only possible if sec has more than one vehicle
                            //Summarize: Swap 
                            //if combined ms of (ivt,ivt2) drops OR 
                            //if ivt2_ms drops OR 
                            //if ivt_ms drops without increasing ivt2_ms
                            //Second condition: Allows for the successive_nodes to find the optimal place in sec
                            //Third condition: sec has more than 1 vehicles and new_max_ms doesn't increase ivt2_ms
                            if( new_max_ms < max_ms - epsilon || 
                                (new_max_ms <= max_ms && ivt2_ms < R->a_VT[ivt2].a_depots[idep].makespan) ||
                                (R->a_VT[ivt2].a_depots[idep].makespan == new_max_ms && ivt_ms < R->a_VT[ivt].a_depots[idep].makespan)
                            ){
                                deleteList(&R->a_VT[ivt].a_depots[idep].routelist);
                                deleteList(&R->a_VT[ivt2].a_depots[idep].routelist);
                                R->a_VT[ivt].a_depots[idep].routelist = copyList(l1);
                                R->a_VT[ivt2].a_depots[idep].routelist = copyList(l2);
                                R->a_VT[ivt].a_depots[idep].makespan = ivt_ms;
                                R->a_VT[ivt2].a_depots[idep].makespan = ivt2_ms;
                                max_ms = new_max_ms;
                                flag = true;
                            }
                        }
                        p = p->next;
                        for(int i = 0; i < listLength(successive_nodes); i++)
                            deleteInPosition(&l2, node_position);
                        if(ivt2 == 2) break;    //Drone doesn't need to move places in ivt2
                    }
                    if(flag){                                                           //Swapped node_data to depot sec
                        //If swap was made, get the updated l1 route
                        deleteList(&l1);
                        l1 = copyList(R->a_VT[ivt].a_depots[idep].routelist);
                        length_l1 = listLength(l1);
                        //node(s) where removed so index_l1 increment is not needed
                    }else{
                        l1_position++;
                        deleteList(&l1);
                        l1 = copyList(R->a_VT[ivt].a_depots[idep].routelist);
                    }
                    deleteList(&l2);
                    deleteList(&successive_nodes);
                }
                //Changing ivt2
                deleteList(&l1);
            }
            //Changing ivt
        }
        //Changing idep
        for(int t = 0; t < G->n_differentTypes; t++)
            remove_duplicate_nodes(&R->a_VT[t].a_depots[idep].routelist);
    }

    R->total_makespan = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        R->a_VT[ivt].makespan = get_makespan_VT(G, &R->a_VT[ivt]);
        if(R->total_makespan < R->a_VT[ivt].makespan)
            R->total_makespan = R->a_VT[ivt].makespan;
    }

    return R->total_makespan;
}

double k_optimization2(route *r, SON *G, VType VT, int n_max){
    if( !r->routelist ||
        !r->routelist->next ||
        listLength(r->routelist) == 3
    ) return r->makespan;

    int list_position = 1, node_position, list_length, i_node, idepot = r->depot_id - G->n_customers - 1;
    double ms_new, ms, ms_best, ms_orig;
    node *successive_nodes = NULL, *p1 = NULL;
    node *p = NULL, *bestroute = NULL;

    node *depotroute = copyList(r->routelist);
    ms = ms_best = ms_orig = r->makespan;
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
            int check = check_route_feasibility(depotroute, G, VT.capacity);
            if(check == 0){
                ms_new = get_makespan_depot_VT(G, depotroute, G->a_depots[idepot].n_VT[VT.IVT], VT.speed);
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
        r->makespan = ms_best;
        //quantity_served remains the same in single colony optimization
    }

    deleteList(&depotroute);
    deleteList(&bestroute);

    remove_duplicate_nodes(&r->routelist);

    return r->makespan;
}

double mutual_k_optimization(vt_solution *R, SON *G, VType VT, int condition, int n_max){

    double ms_og = R->makespan;

    node *successive_nodes = NULL, *l1 = NULL, *l2 = NULL, *p1 = NULL;

    for(int i = 0; i < G->n_depots; i++)
        push(&R->a_depots[i].routelist, R->a_depots[i].routelist->data);

    //Begin mutual colony optimization
    int idep = 0, sec;
    while(idep < G->n_depots){
        if(G->a_depots[idep].n_VT[R->IVT] == 0 || !R->a_depots[idep].routelist->next){idep++; continue;}
        sec = 0;
        while(sec < G->n_depots){
            if( idep == sec || 
                G->a_depots[sec].n_VT[R->IVT] == 0 ||
                !R->a_depots[sec].routelist->next
            ){ sec++; continue; }
            l1 = copyList(R->a_depots[idep].routelist);
            int length_l1 = listLength(l1);
            int l1_position = 1, i_node;
            // move nodes in route
            while(l1_position < length_l1-n_max){                                   //For every node(s) in l1
                //Get ms of the two depots
                double max_ms;
                if(R->a_depots[idep].makespan < R->a_depots[sec].makespan - epsilon){
                    max_ms = R->a_depots[sec].makespan;
                }else{
                    max_ms = R->a_depots[idep].makespan;
                }
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

                double idep_ms = get_makespan_depot_VT(G, l1, G->a_depots[idep].n_VT[R->IVT], VT.speed);
                bool flag = false;

                deleteList(&l2);
                l2 = copyList(R->a_depots[sec].routelist);
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
                    int check = check_route_feasibility(l2, G, VT.capacity);
                    if(check == 0){

                        double sec_ms = get_makespan_depot_VT(G, l2, G->a_depots[sec].n_VT[R->IVT], VT.speed);

                        if(condition == 1){
                            if(idep_ms < R->a_depots[idep].makespan && sec_ms < R->a_depots[sec].makespan){
                                //Test later
                            }
                        }else if(condition == 2){
                            double new_max_ms;
                            if(idep_ms < sec_ms - epsilon){
                                new_max_ms = sec_ms;
                            }else{
                                new_max_ms = idep_ms;
                            }
                            //Accept swap if
                            //new_max_ms < max_ms || (new_max_ms <= max_ms && sec_ms < R->a_depots[sec].makespan)
                            //if max_ms is idep_ms then max_ms cannot change while moving nodes in sec
                            //if max_ms is sec_ms then swap is only possible if sec has more than one vehicle
                            //so the swap happens to the route of the vehicle with minimum time
                            //Summarize: Swap if combined ms of (idep,sec) drops OR if sec_ms drops OR if idep_ms drops
                            //Second condition: Allows for the successive_nodes to find the optimal place in sec
                            //Third condition: sec has more than 1 vehicles and new_max_ms doesn't increase ms
                            if( new_max_ms < max_ms - epsilon || 
                                (new_max_ms <= max_ms && sec_ms < R->a_depots[sec].makespan) ||
                                (R->a_depots[sec].makespan == new_max_ms && idep_ms < R->a_depots[idep].makespan)
                            ){
                                deleteList(&R->a_depots[idep].routelist);
                                deleteList(&R->a_depots[sec].routelist);
                                R->a_depots[idep].routelist = copyList(l1);
                                R->a_depots[sec].routelist = copyList(l2);
                                R->a_depots[idep].makespan = idep_ms;
                                R->a_depots[sec].makespan = sec_ms;
                                max_ms = new_max_ms;
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

    R->makespan = get_makespan_VT(G, R);

    return R->makespan;
}


double mutual_drone(vt_solution *R, SON *G, VType VT){

    double ms_og = R->makespan;

    int idep = 0, sec;
    node *l1 = NULL, *l2 = NULL;
    //Begin mutual colony optimization
    while(idep < G->n_depots){
        if( G->a_depots[idep].n_VT[R->IVT] == 0 || 
            !R->a_depots[idep].routelist->next ||
            listLength(R->a_depots[idep].routelist) == 3
        ){
            idep++; 
            continue;
        }
        int sec = 0;
        while(sec < G->n_depots){
            if( idep == sec || 
                G->a_depots[sec].n_VT[R->IVT] == 0 ||
                R->a_depots[idep].makespan < R->a_depots[sec].makespan - epsilon    //No point in running
            ){ 
                sec++; 
                continue; 
            }
            deleteList(&l2);
            l2 = copyList(R->a_depots[sec].routelist);
            deleteList(&l1);
            l1 = copyList(R->a_depots[idep].routelist);
            int length_l1 = listLength(l1);
            int l1_position = 1, i_node;
            // move nodes in route
            while(l1_position < length_l1-1){                                   //For every node(s) in l1
                //Get ms of the two depots
                double max_ms;
                if(R->a_depots[idep].makespan < R->a_depots[sec].makespan - epsilon){
                    deleteList(&l1);
                    break;  //No swaps possible, move to next sec
                }else{
                    max_ms = R->a_depots[idep].makespan;
                }
                // Get i_node to move from idep
                i_node = getNth(R->a_depots[idep].routelist, l1_position);
                while(i_node > G->n_customers){                                 //Don't move depot node(s)
                    l1_position++;
                    i_node = getNth(R->a_depots[idep].routelist, l1_position);  //Get node(s) value
                }
                //if i_node out of bounds break?
                if(i_node < 1 || i_node > G->n_customers+1) break;
             
                //Delete i_node and depot node from l1 
                deleteInPosition(&l1, l1_position+1);
                deleteInPosition(&l1, l1_position+1);

                //push node_data and depot to sec routelist
                push(&l2, i_node);
                push(&l2, G->a_depots[sec].id);
                
                double idep_ms = get_makespan_depot_VT(G, l1, G->a_depots[idep].n_VT[R->IVT], VT.speed);
                double sec_ms = get_makespan_depot_VT(G, l2, G->a_depots[sec].n_VT[R->IVT], VT.speed);

                double new_max_ms;
                if(idep_ms < sec_ms - epsilon){
                    new_max_ms = sec_ms;
                }else{
                    new_max_ms = idep_ms;
                }

                if(new_max_ms < max_ms - epsilon){
                    deleteList(&R->a_depots[idep].routelist);
                    deleteList(&R->a_depots[sec].routelist);
                    R->a_depots[idep].routelist = copyList(l1);
                    R->a_depots[sec].routelist = copyList(l2);
                    R->a_depots[idep].makespan = idep_ms;
                    R->a_depots[sec].makespan = sec_ms;
                    length_l1 -= 2;
                    //l1 and l2 stay in their current state
                }else{
                    //Reset l1
                    deleteList(&l1);
                    l1 = copyList(R->a_depots[idep].routelist);
                    //Reset l2
                    deleteInPosition(&l2, 1);
                    deleteInPosition(&l2, 1);
                    l1_position++;
                }
            }
            sec++;
            deleteList(&l2);
        }
        idep++;
        deleteList(&l1);
    }

    R->makespan = get_makespan_VT(G, R);
    return R->makespan;
}

void remove_duplicate_nodes(node **combined_list){
    
    node *current = *combined_list, *temp = NULL;
    if(!current){perror("current == NULL at remove_duplicate_nodes at LocalOptimization\n"); }
    
    while(current && current->next){
        if(current->data == current->next->data){                   //If duplicate
            temp = current->next;
            current->next = current->next->next;
            free(temp);
            //continue;   //Check for another duplicate (3 or more in a row)
        }else{
            current = current->next;
        }
    }
    return;
}

int check_route_feasibility(node *route, SON *G, int capacity){
    node *p = route;
    int limit = 0;
    while(p){
        if(p->data <= G->n_customers && p->data > 0){
            limit += G->a_customers[p->data-1].demand;
            if(limit > capacity)
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
