#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "header_files/structs.h"

void offload_to_motorbikes(SON *G, VType *VT, asolution *R_best, vt_solution *Rt_best, int **da_access, int m_capacity){
    for(int i = 0; i < G->n_depots; i++){
        int route_customers = 0;    //Keeps track of the number of customers the motorbike serves
        node *p = Rt_best->a_depots[i].routelist;
        while(p && p->next){
            bool drone = false;
            double drone_prob = -1.0;
            if(p->data <= G->n_customers && da_access[0][p->data - 1] == 0){
                if(da_access[2][p->data - 1] != 0 && VT[2].n_vehicles > 0){ //If accessible by drone and drones exist
                    drone_prob = (double)VT[2].n_vehicles / (VT[2].n_vehicles + VT[1].n_vehicles);
                    if((double)random() / RAND_MAX < drone_prob){
                        drone = true;
                    }
                }
                if(!drone){
                    if(G->a_depots[i].n_VT[1] != 0){                                    //checks if depot has motorbike
                        append(&R_best->a_VT[1].a_depots[i].routelist, p->data);        //Add customer to the motorbike route
                        deleteOnKey(&R_best->a_VT[0].a_depots[i].routelist, p->data);   //Delete customer from the Truck route
                        //Check if motorbike needs to return to the depot
                        route_customers++;
                        if(route_customers == m_capacity){
                            append(&R_best->a_VT[1].a_depots[i].routelist, R_best->a_VT[1].a_depots[i].depot_id);
                            route_customers = 0;
                        }
                    }else{
                        //If Depot doesn't have a Motorbike then assign to the closest depot that has one
                        double min_distance = HUGE_VAL;
                        int idep = -1;
                        for(int j = 0; j < G->n_depots; j++){
                            if(G->a_depots[j].n_VT[1] != 0){
                                if(G->d_matrix[G->a_depots[j].id - 1][p->data - 1] < min_distance){
                                    min_distance = G->d_matrix[G->a_depots[j].id - 1][p->data - 1];
                                    idep = j;
                                }
                            }
                        }
                        //Create separate route for customers of other depots' customers
                        if(route_customers == 0){
                            append(&R_best->a_VT[1].a_depots[idep].routelist, G->a_depots[idep].id);
                        }
                        append(&R_best->a_VT[1].a_depots[idep].routelist, p->data);         //Add customer to the motorbike route
                        deleteOnKey(&R_best->a_VT[0].a_depots[i].routelist, p->data);       //Delete customer from the Truck route
                        //Check if motorbike needs to return to the depot
                        route_customers++;
                        if(route_customers % m_capacity == 0){
                            append(&R_best->a_VT[1].a_depots[idep].routelist, R_best->a_VT[1].a_depots[idep].depot_id);
                        }
                    }
                }else{  //if drone == true
                    if(G->a_depots[i].n_VT[2] != 0){                                    //checks if depot has drone
                        append(&R_best->a_VT[2].a_depots[i].routelist, p->data);        //Add customer to the drone route
                        append(&R_best->a_VT[2].a_depots[i].routelist, G->a_depots[i].id);        
                        deleteOnKey(&R_best->a_VT[0].a_depots[i].routelist, p->data);   //Delete customer from the Truck route
                    }else{//assign to the closest depot that has drone
                        double min_distance = HUGE_VAL;
                        int idep = -1;
                        for(int j = 0; j < G->n_depots; j++){
                            if(G->a_depots[j].n_VT[2] != 0){
                                if(G->d_matrix[G->a_depots[j].id - 1][p->data - 1] < min_distance){
                                    min_distance = G->d_matrix[G->a_depots[j].id - 1][p->data - 1];
                                    idep = j;
                                }
                            }
                        }
                        append(&R_best->a_VT[2].a_depots[idep].routelist, p->data);        //Add customer to the drone route
                        append(&R_best->a_VT[2].a_depots[idep].routelist, G->a_depots[idep].id);        
                        deleteOnKey(&R_best->a_VT[0].a_depots[i].routelist, p->data);   //Delete customer from the Truck route
                    }
                }
            }
            p = p->next;
        }
        if(route_customers % m_capacity != 0)
            append(&R_best->a_VT[1].a_depots[i].routelist, R_best->a_VT[1].a_depots[i].depot_id);   //Finish motorbike route
    }
    return;
}
