#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include "header_files/heuristic_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/structs.h"

extern int instance_id;

double get_makespan_depot_VT(SON *G, node *routelist, int n_vehicles, double speed){
    if(!routelist)
        return 0;
    double ms = 0.0, route_time = 0.0, *r_times = NULL;
    int n_routes = 0;
    node *p = routelist;
    while(p->next){
        route_time += G->d_matrix[p->data - 1][p->next->data - 1] / speed;
        if(p->next->data > G->n_customers){
            n_routes++;
            double *temp;
            if(NULL == (temp = realloc(r_times, n_routes * sizeof *r_times))){
                printf("Error reallocing r_times\n");
                exit(1);
            }
            r_times = temp;
            r_times[n_routes - 1] = route_time;
            route_time = 0.0;
        }
        p = p->next;
    }
    //Sort r_times in descending order
    quickSort(r_times, 0, n_routes-1);
    //Assign routes to vehicles
    double v_times[n_vehicles];
    for(int i = 0; i < n_vehicles; i++)
        v_times[i] = 0;

    for(int r = 0; r < n_routes; r++){
        int min_index = 0;
        for(int i = 1; i < n_vehicles; i++){
            if(v_times[i] < v_times[min_index] - epsilon){
                min_index = i;
            }
        }
        v_times[min_index] += r_times[r];
    }
    free(r_times);
    //Get the max route of this depot
    ms = v_times[0];
    for(int v = 1; v < n_vehicles; v++){
        if(v_times[v] - epsilon > ms)
            ms = v_times[v];
    }
    return ms;
}

double get_makespan_VT(SON *G, vt_solution *R){
    double ms = 0.0;
    for(int idep = 0; idep < G->n_depots; idep++){
        if(G->a_depots[idep].n_VT[R->IVT] != 0){
            if(ms < R->a_depots[idep].makespan - epsilon)
                ms = R->a_depots[idep].makespan;
        }
    }
    return ms;
}

double get_total_makespan(asolution *R, int differentTypes){
    double ms = 0.0;
    for(int i = 0; i < differentTypes; i++){
        if(ms < R->a_VT[i].makespan)
            ms = R->a_VT[i].makespan;
    }
    return ms;
}


//Used to assign customers that remained on the Truck route to their closest depot
int find_closest_depot(int customerID, SON *G, int **da_access){
    if(!da_access || !*da_access || !G){
        perror("Error in find_closest_depot!\n");
        exit(1);
    }
    int depID = -1;
    double d_min = HUGE_VAL;
    //Get closest depot that CAN serve customer
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        if(da_access[ivt][customerID - 1] == 1){
            for(int i = 0; i < G->n_depots; i++){
                if(G->a_depots[i].n_VT[ivt] != 0){
                    double d = G->d_matrix[customerID - 1][G->a_depots[i].id - 1];
                    if(d < d_min){
                        d_min = d;
                        depID = G->a_depots[i].id;
                    }
                }
            }
        }
    }
    if(depID == -1){
        printf("find_closest_depot returns -1 for customer_id %d\n", customerID);
        exit(1);
    }
    return depID;
}

//Try this
double get_time_VT(node *route, SON *G, double speed){
    double ms = 0.0;
    node *p = route;
    while(p && p->next){
        ms += G->d_matrix[p->data - 1][p->next->data - 1] / speed;
        p = p->next;
    }
    return ms;
}

adj_node *create_adj_list(int nodeID, SON *G, int *arr, int n){
    adj_node *head = NULL;
    for(int i = 0; i < n; i++){
        if(arr[i] != nodeID){
            double d = G->d_matrix[nodeID - 1][arr[i] - 1];
            adj_add_node(&head, arr[i], d);
        }
    }
    return head;
}


bool not_empty(int *array, int length){
    if(!array){ perror("array == NULL at not_empty()\n"); exit(1); }
    for(int i = 0; i < length; i++){
        if(array[i] == 1){
            return true;
        }
    }
    return false;
}


void fprint_results(asolution *R, SON *G, VType *VT){

    int detect_dup[G->n_customers];
    for(int i = 0; i < G->n_customers; i++)
        detect_dup[i] = 0;

    char file_name[20], fn[30];
    sprintf(file_name, "prox%02d.res", instance_id);
    FILE *fp, *fp_1;
    if(NULL == (fp = fopen(file_name, "w")))
    {
        perror("Couldn't open file fp at fprint_results\n");
        exit(1);
    }
    fprintf(fp, "total makespan: %.2lf\n", R->total_makespan);

    node *temp = NULL, *vehicleRoute = NULL;
    push(&vehicleRoute, 0);
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        double ivt_ms = 0.0;
        sprintf(fn, "prox%02d_ivt_%d.csv", instance_id, ivt+1);
        if(NULL == (fp_1 = fopen(fn, "w"))){
            perror("Error opening fp_1!\n");
            exit(1);
        }
        fprintf(fp_1, "StartNode,EndNode\n");
        for(int idep = 0; idep < G->n_depots; idep++){
            if(G->a_depots[idep].n_VT[ivt] != 0){
                double dep_ms = 0.0;
                int vehicle = 1;
                int q_served = 0;
                if(!R->a_VT[ivt].a_depots[idep].routelist){
                    continue;
                } 
                temp = R->a_VT[ivt].a_depots[idep].routelist;
                double v_ms = 0.0;
                while(temp->next){
                    v_ms += G->d_matrix[temp->data - 1][temp->next->data - 1] / VT[ivt].speed;
                    if(temp->next->data > G->n_customers){
                        append(&vehicleRoute, 0);
                        fprintf(fp, "type %d depot %d route %d q_served %d time %0.2lf\t", ivt+1, idep+G->n_customers+1, vehicle, q_served, v_ms);
                        fprintf(fp_1, "%d,%d\n", temp->data, temp->next->data);
                        fprintList(vehicleRoute, fp);
                        deleteList(&vehicleRoute);
                        vehicle++;	
                        q_served = 0;
                        push(&vehicleRoute, 0);
                        dep_ms += v_ms;
                        v_ms = 0.0;
                    }else{
                        q_served += G->a_customers[temp->next->data-1].demand;
                        append(&vehicleRoute, temp->next->data);
                        detect_dup[temp->next->data - 1]++;
                        fprintf(fp_1, "%d,%d\n", temp->data, temp->next->data);
                    }
                    temp = temp->next;
                }
                if(dep_ms > ivt_ms)
                    ivt_ms = dep_ms;
            }
        }
        fprintf(fp, "type %d makespan %0.2lf\nivt_ms %0.3lf\n", ivt+1, R->a_VT[ivt].makespan, ivt_ms);
        if(fclose(fp_1) != 0){ 
            perror("Error closing fp_1\n"); 
            exit(1); 
        }
    }
    bool flag = false;
    for(int i = 0; i < G->n_customers; i++){
        if(detect_dup[i] != 1){
            flag = true;
            printf("detect_dup[%d] = %d\n", i, detect_dup[i]);
        }
    }
    if(flag)
        exit(1);

    if(fclose(fp) != 0){ 
        perror("Couldn't close fp\nExiting...\n"); 
        exit(1); 
    }

    free(vehicleRoute);

    return;
}

void fprint_data(double runtime){
    char file_name[20];
    sprintf(file_name, "prox%02d.data", instance_id);
    FILE *fp;
    if(NULL == (fp = fopen(file_name, "w"))){ 
        perror("Couldn't open file fp at fprint_results\n"); 
        exit(1); 
    }

    fprintf(fp, "Total runtime: %.2lf seconds\n", runtime);
    
    if(fclose(fp) != 0){ 
        perror("fclose error at fprint_data\n"); 
        exit(1); 
    }
    
    return;
}
//
//Adds customer to pre-existing depot route if possible
//To the route to which it adds the least time
int addToRoute(SON *G, node **route, int customerID, int capacity, double speed){
    printf("addToRoute %d\n", customerID);
    printList(*route);
    int added = 0;
    if(!*route) return added;
    node *p = (*route)->next, *prev = p, *min_p = NULL;
    int load = 0;
    double curr, min = HUGE_VAL;
    while(p){
        printf("p = %d\t prev->data = %d\n", p->data, prev->data);
        if(p->data <= G->n_customers){
            load += G->a_customers[p->data-1].demand;
        }else if(p->data > G->n_customers){
            if(load + G->a_customers[customerID-1].demand <= capacity){
                curr = 0.0;
                curr += G->d_matrix[prev->data - 1][customerID - 1];
                curr += G->d_matrix[customerID - 1][p->data - 1];
                if(curr < min - epsilon){
                    min = curr;
                    min_p = prev;
                }
                added = 1;
            }
            load = 0;
        }
        prev = p; 
        p = p->next;
    }
    if(added == 1)
        insertAfterNode(min_p, customerID);
    printf("After:\n");
    printList(*route);

    return added;
}

//quickSort
void swap(double *p1, double *p2){
    double temp;
    temp = *p1;
    *p1 = *p2;
    *p2 = temp;
}

int partition(double *arr, int low, int high){
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] > pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

//high should be size of array - 1
void quickSort(double *arr, int low, int high){
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

