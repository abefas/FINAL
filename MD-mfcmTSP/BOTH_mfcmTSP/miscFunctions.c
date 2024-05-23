#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/structs.h"

double calculate_pheromone_sum(int ilast, node *v_cand, double *phMatrix, int idep, int n_nodes){
    if(!v_cand){ perror("v_cand == NULL at calculate_pheromone_sum\n"); exit(1); }
    if(!phMatrix){ perror("phMatrix == NULL at calculate_pheromone_sum\n"); exit(1); }
    double sum = 0.0;
    node *temp = v_cand;

    while(temp){
        if(!phMatrix[idep*(n_nodes)*(n_nodes) + ilast*(n_nodes) + (temp->data-1)]) perror("phMatrix NULL!\n");
        if(phMatrix[idep*(n_nodes)*(n_nodes) + ilast*(n_nodes) + (temp->data-1)] == -1.0){
            perror("calculate_pheromone_sum error!\n");
            exit(1);
        }
        sum += phMatrix[idep*(n_nodes)*(n_nodes) + ilast*(n_nodes) + (temp->data - 1)];
        temp = temp->next;
    }

    return sum;
}

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
        if (arr[j] > pivot) {   //descending order
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

double calculate_h(SON *G, node *v_cand, int ilast){
    if(!G){ perror("G == NULL at calculate_h\n"); exit(1); }
    if(!v_cand){ perror("v_cand == NULL at calculate_h\n"); exit(1); }
    double distance = 0.0;
    struct Node *temp = v_cand;
    while(temp){
        distance += G->d_matrix[ilast][temp->data - 1];
        temp = temp->next;
    }
    return 1.0/distance;
}

//Calculate arctan (inverse tangent)
double calculate_angle(int x, int y){
    return atan2(y,x) * (180.0/M_PI);   //* (180.0/M_PI) is used to transform radians to degrees
}

void find_polar_coordinates(SON *G, int vi, polar *polarArray){
    if(!polarArray){ perror("polarArray == NULL at find_polar_coordinates\n"); exit(1); }
    //Cluster Vertex is out of bounds
    if(vi < 0 || vi >= G->n_nodes){
        perror("Error in findPolarCoordinates in CreateClusters\n");
        exit(1);
    }
    // Assign Cluster Vertex as origin point instead of 0,0 in the cartesian plane 
    float x0, y0;
    x0 = G->a_combined[vi].x;
    y0 = G->a_combined[vi].y;

    float dx, dy;
    for(int i = 0; i < G->n_customers; i++){
        //If Cluster Vertex
        if(i == vi){
            polarArray[i].distance = -1;
            continue;
        //If not Cluster Vertex then find polar coordinates based on Cluster Vertex position
        }else{
            dx = G->a_combined[i].x - x0;
            dy = G->a_combined[i].y - y0;
            polarArray[i].distance = sqrt((dx*dx) + (dy*dy));
            polarArray[i].degrees = calculate_angle(dx, dy);
            
            if(polarArray[i].degrees < 0.0)     //Transform negative angle
                polarArray[i].degrees += 360.0;
        }
    }
    return;
}

void assign_vertices_to_sectors(SON *G, int vi, polar *polarArray, sector *sec, int n_sect){
    if(!polarArray){ perror("polarArray == NULL at assign_vertices_to_sectors\n"); exit(1); }
    if(!sec){ perror("sec == NULL at assign_vertices_to_sectors\n"); exit(1); }
    //Initialize sectors
    for(int k = 0; k < n_sect; k++)
        sec[k].vertexList = NULL;
    
    //Assign nodes to sectors
    for(int i = 0; i < G->n_customers; i++){
        int sc = 0; //sector index [0, n_sect-1]
        if(i == vi) //Cluster Vertex is not in any sector
            continue;
        
        for(double j = 360.0/n_sect; j <= 360.0; j += 360.0/n_sect){
            if(polarArray[i].degrees <= j){
                append(&sec[sc].vertexList, G->a_customers[i].id);   // Add vertex id to its sector
                break;
            }
            sc++;
        }
    }
    return;
}

bool not_empty_clusters(int *array, int length, int *da_access){
    if(!array){ perror("array == NULL at not_empty()\n"); exit(1); }
    for(int i = 0; i < length; i++){
        if(array[i] == 1 && da_access[i] == 1){
            return true;
        }
    }
    return false;
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

bool search_cluster_VT(int *K_v, int icluster, int *v_free, int n_size){
    if(!K_v){ perror("K_v == NULL at search_cluster in misc_functions\n"); exit(1); }
    for(int i = 0; i < n_size; i++){ 
        if(K_v[(icluster*n_size) + i] > 0 && v_free[K_v[(icluster*n_size) + i] - 1] == 1)
            return true;
    }
    return false;
}

int find_closest_free_vertex_in_sector(polar *polarArray, node *vertexList, int *da_access){
    if(!polarArray){ perror("polarArray == NULL at find_closest_free_vertex_in_sector\n"); exit(1); }
    if(!vertexList) { perror("vertexList == NULL at find_closest_free_vertex_in_sector()"); exit(1); }   //if sector j is empty return -1
    node *temp = vertexList; 
    double minDistance = HUGE_VAL;
    int minID = -1;
    //Search sector for closest vertex (At this point all vertices are still free anyway)
    while(temp){
        if(polarArray[temp->data - 1].distance < minDistance - epsilon && da_access[temp->data - 1] == 1){   //Compare vertices in sector
            minDistance = polarArray[temp->data - 1].distance;
            minID = temp->data;
        }
        temp = temp->next;
    }     
    return minID;    //return ID of closest vertex in sector j
}

void find_free_in_clusterk_VT(node **head, int *v_free, int *K_v, int icluster, int n_size){
    if(!v_free){ perror("v_free == NULL at find_free_in_cluster_k\n"); exit(1); }
    if(!K_v){ perror("K_pos == NULL at find_free_in_cluster_k\n"); exit(1); }
    if(!K_v[icluster*n_size]) return;
    for(int j = 0; j < n_size; j++){
        //If vertex is customer ID and is free then push into v_cand
        if(K_v[(icluster * n_size) + j] > 0){
            if(v_free[K_v[(icluster * n_size) + j] - 1] == 1)
                push(head, K_v[(icluster * n_size) + j]);
        }
    }
   return; 
}

int find_closest_free_vertex(int *v_free, polar *polarArray, int n_customers, int *da_access){
    if(!v_free){ perror("v_free == NULL at find_closest_free_vertex\n"); exit(1); }
    if(!polarArray){ perror("polarArray == NULL at find_closest_free_vertex\n"); exit(1); }
    double minDistance = HUGE_VAL;
    int minID = -1;
    for(int i = 0; i < n_customers; i++){
        if(v_free[i] == 1 && polarArray[i].distance < minDistance - epsilon && da_access[i] == 1){
            minDistance = polarArray[i].distance;
            minID = i+1;
        }
    }
    if(minID < 1){
        printf("minID < 1\n");
        exit(1);
    }
    return minID;       //return ID of closest unvisited vertex (no matter the sector)
}

int errCheck(int value, int lower, int upper){
    if(value < lower || value > upper)
        return 1;
    return 0;
}

void new_best(vt_solution *new_r, vt_solution *r, SON *G){
    for(int idep = 0; idep < G->n_depots; idep++){
        if(G->a_depots[idep].n_VT[r->IVT] != 0){
            new_r->a_depots[idep].makespan = r->a_depots[idep].makespan;
            new_r->a_depots[idep].quantity_served = r->a_depots[idep].quantity_served;
            deleteList(&new_r->a_depots[idep].routelist);                                         //Delete old 
            new_r->a_depots[idep].routelist = copyList(r->a_depots[idep].routelist);
        }
    }
    new_r->makespan = r->makespan;
    return;
}

int store_edge_count(vt_solution *Ra, SON *G, int *edge_matrix){

    if(!edge_matrix){ perror("p == NULL at store_edge_count\n"); exit(1); }
    int edge_sum = 0;

    for(int idep = 0; idep < G->n_depots; idep++){
        if(G->a_depots[idep].n_VT[Ra->IVT] != 0){
            node *temp = Ra->a_depots[idep].routelist;
            if(!temp){perror("temp == NULL at function store_edge_count...\n"); exit(1);}
            while(temp && temp->next){
                if(temp->data < 1 || temp->data > G->n_nodes || temp->next->data < 1 || temp->next->data > G->n_nodes){
                    perror("Wrong values at temp->data at store_edge_count\n");
                    exit(1);
                }
                edge_matrix[(G->n_nodes)*(temp->data - 1) + (temp->next->data - 1)] += 1;
                edge_sum++;

                temp = temp->next;
            }
        } 
    }
    return edge_sum;
}


void update_pheromone(SON *G, vt_solution *R, vt_solution *R_best, double *phMatrix, double T_update, double d){
    
    if(!phMatrix){ perror("phMatrix == NULL at update_pheromone\n"); exit(1); }
    
    double probability_R_best, t, x, R_update; 
    bool flag = false;

    if(R->makespan < R_best->makespan - epsilon){           
        t = (R_best->makespan - R->makespan)/R->makespan;
        probability_R_best = exp(-t/T_update);                    
        //printf("T_update = %0.2lf\n", T_update);
        //printf("probability_R_best = %0.3lf\n", probability_R_best);
        if((double)random()/(double)RAND_MAX < probability_R_best - epsilon){
            R_update = R_best->makespan;                          //Use worse solution R_best
            flag = true;
        }else{
            R_update = R->makespan;
        }
    }else{                                                          
        R_update = R->makespan;                                   //In this case R == R_best
    }

    node *temp = NULL;
    for(int idep = 0; idep < G->n_depots; idep++){
        if(G->a_depots[idep].n_VT[R->IVT] != 0){
            if(flag){
                temp = R_best->a_depots[idep].routelist;
            }else{
                temp = R->a_depots[idep].routelist;
            }
            if(!temp){ perror("temp = NULL at update_pheromone \n"); continue; }
            while(temp && temp->next){
                if(phMatrix[idep*G->n_nodes*G->n_nodes + (temp->data-1)*G->n_nodes + temp->next->data - 1] < 0.0){
                    perror("pheromone update error\n"); 
                    printf("routes:\nR:\n");
                    printList(R->a_depots[idep].routelist);
                    printf("\nR_best:\n");;
                    printList(R_best->a_depots[idep].routelist);
                    printf("\n");
                    printf("temp %d temp->next %d\n", temp->data, temp->next->data);
                    printf("phMatrix = %0.2lf\n", 
                           phMatrix[idep*G->n_nodes*G->n_nodes + (temp->data-1)*G->n_nodes + temp->next->data-1]);
                    exit(1);
                }

                phMatrix[idep*G->n_nodes*G->n_nodes + (temp->data-1)*G->n_nodes + (temp->next->data-1)] += 
                    d*(R->makespan/R_update);

                //Symmetrical pheromone update for symmetrical graphs
                phMatrix[idep*G->n_nodes*G->n_nodes + (temp->next->data-1)*G->n_nodes + (temp->data-1)] += 
                    d*(R->makespan/R_update);

                temp = temp->next;
            }
        }
    }
    return;
}

double evaporate_pheromones(SON *G, double *phMatrix, int *edge_matrix, int *da_access, int edge_sum, int n_ants, double p_min, double p_max){
    
    if(!phMatrix){ perror("phMatrix == NULL at evaporate_pheromones\n"); exit(1); }
    if(!edge_matrix){ perror("edge_matrix == NULL at evaporate_pheromones\n"); exit(1); }

    double H = 0.0, *p;
    if(NULL == (p = calloc(G->n_nodes*G->n_nodes, sizeof *p))){
        printf("Error callocing p[]\n");
        exit(1);
    }
    
    //Upper triangle of matrix used
    for(int i = 1; i < G->n_nodes; i++){
        for(int j = 0; j < i; j++){
            if(i != j && da_access[i] == da_access[j] == 1){
                p[(G->n_nodes)*i + j] = (double) edge_matrix[(G->n_nodes)*i + j] / edge_sum +
                                        (double) edge_matrix[(G->n_nodes)*j + i] / edge_sum;
            }
        }
    }

    for(int i = 1; i < G->n_nodes; i++){
        for(int j = 0; j < i; j++){
            if(i != j && da_access[i] == da_access[j] == 1){
                if(p[(G->n_nodes)*i + j] > 0)
                    H -= p[(G->n_nodes)*i + j] * log2(p[(G->n_nodes)*i + j]);
            }
        }
    }
    free(p);

    double H_min, H_max;
    H_min = -log2((double)n_ants/edge_sum);
    H_max = -log2(1.0/edge_sum);

    double rho = p_min + (p_max - p_min)*((H - H_min)/(H_max - H_min));

    for(int idep = 0; idep < G->n_depots; idep++){
        for(int c1 = 1; c1 < G->n_nodes; c1++){
            for(int c2 = 0; c2 < c1; c2++){
                if( phMatrix[idep*G->n_nodes*G->n_nodes + G->n_nodes*c1 + c2] != -1.0 &&
                    phMatrix[idep*G->n_nodes*G->n_nodes + G->n_nodes*c1 + c2] > 0.0001)
                {
                    phMatrix[idep*G->n_nodes*G->n_nodes + G->n_nodes*c1 + c2] *= (1 - rho);
                }

                if( phMatrix[idep*G->n_nodes*G->n_nodes + G->n_nodes*c2 + c1] != -1.0 && 
                    phMatrix[idep*G->n_nodes*G->n_nodes + G->n_nodes*c2 + c1] > 0.0001)
                {
                    phMatrix[idep*G->n_nodes*G->n_nodes + G->n_nodes*c2 + c1] *= (1 - rho);
                }
            }
        }
    }

    printf("rho = %0.3lf\n", rho);
    return (H-H_min)/H_min; //Termination condition
}


extern int version_result;
extern int instance_id;
extern int AACORUN;

void fprint_results(asolution *R, SON *G, VType *VT, int **da_access){

    int detect_dup[G->n_customers];
    for(int i = 0; i < G->n_customers; i++)
        detect_dup[i] = 0;

    char file_name[30], fn[30];
    sprintf(file_name, "p%02d-%d_v%d.res", instance_id, AACORUN, version_result);

    FILE *fp, *fp_1;
    if(NULL == (fp = fopen(file_name, "w")))
    {
        perror("Couldn't open file fp at fprint_results\n");
        exit(1);
    }

    fprintf(fp, "total makespan: %0.2lf\n", R->total_makespan);
    node *temp = NULL, *vehicleRoute = NULL;
    push(&vehicleRoute, 0);
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        sprintf(fn, "p%02d-%d_%d_ivt_%d.csv", instance_id, AACORUN, version_result, ivt+1);
        double ivt_ms = 0.0, dep_ms = 0.0;
        if(NULL == (fp_1 = fopen(fn, "w"))){
            perror("Error opening fp_1!\n");
            exit(1);
        }
        fprintf(fp_1, "StartNode,EndNode\n");
        for(int idep = 0; idep < G->n_depots; idep++){
            if(G->a_depots[idep].n_VT[ivt] != 0){
                dep_ms = 0.0;
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
                        fprintf(fp, "%d  %d  r %2d  q_s %d  time %10.2lf\t", ivt+1, idep+G->n_customers+1, vehicle, q_served, v_ms);
                        fprintf(fp_1, "%d,%d\n", temp->data, temp->next->data);
                        fprintList(vehicleRoute, fp);
                        deleteList(&vehicleRoute);
                        vehicle++;	
                        q_served = 0;
                        push(&vehicleRoute, 0);
                        dep_ms += v_ms;
                        v_ms = 0.0;
                    }else{
                        if(da_access[ivt][temp->next->data - 1] != 1){
                            fprintf(fp, "customer %d cannot be served by this type (%d)\n", temp->data, ivt);
                        }
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
        fprintf(fp, "type %d makespan %0.2lf\nfprint ivt_ms %0.3lf\n", ivt+1, R->a_VT[ivt].makespan, ivt_ms);
        if(fclose(fp_1) != 0){
            perror("Error closing fp_1!\n");
            exit(1);
        }
    }
    for(int i = 0; i < G->n_customers; i++){
        if(detect_dup[i] != 1){
            fprintf(fp, "detect_dup[%d] = %d\n", i, detect_dup[i]);
        }
    }

    if(fclose(fp) != 0){
        perror("Error closing fp!\n");
        exit(1);
    }

    free(vehicleRoute);

    return;
}

void fprint_results_VT(vt_solution *R, SON *G, VType VT, int *da_access){

    int detect_dup[G->n_customers];
    for(int i = 0; i < G->n_customers; i++)
        detect_dup[i] = 0;

    char file_name[30], fn[30];
    sprintf(file_name, "p%02d-%d_AACONC.res", instance_id, AACORUN);

    FILE *fp, *fp_1;
    if(NULL == (fp = fopen(file_name, "w")))
    {
        perror("Couldn't open file fp at fprint_results\n");
        exit(1);
    }

    fprintf(fp, "total makespan: %0.2lf\n", R->makespan);
    node *temp = NULL, *vehicleRoute = NULL;
    push(&vehicleRoute, 0);
    sprintf(fn, "p%02d-%d_AACONC.csv", instance_id, AACORUN);
    if(NULL == (fp_1 = fopen(fn, "w"))){
        perror("Error opening fp_1!\n");
        exit(1);
    }
    fprintf(fp_1, "StartNode,EndNode\n");
    double distance = 0.0;
    for(int idep = 0; idep < G->n_depots; idep++){
        if(G->a_depots[idep].n_VT[R->IVT] != 0){
            int vehicle = 1;
            int q_served = 0;
            if(!R->a_depots[idep].routelist){
                continue;
            } 
            temp = R->a_depots[idep].routelist;
            double v_ms = 0.0;
            while(temp->next){
                v_ms += G->d_matrix[temp->data - 1][temp->next->data - 1] / VT.speed;
                distance += G->d_matrix[temp->data - 1][temp->next->data - 1];
                if(temp->next->data > G->n_customers){
                    append(&vehicleRoute, 0);
                    fprintf(fp, "type %d depot %d route %d q_served %d time %0.2lf\t", R->IVT+1, idep+G->n_customers+1, vehicle, q_served, v_ms);
                    fprintf(fp_1, "%d,%d\n", temp->data, temp->next->data);
                    fprintList(vehicleRoute, fp);
                    deleteList(&vehicleRoute);
                    vehicle++;	
                    q_served = 0;
                    push(&vehicleRoute, 0);
                    v_ms = 0.0;
                }else{
                    q_served += G->a_customers[temp->next->data-1].demand;
                    append(&vehicleRoute, temp->next->data);
                    detect_dup[temp->next->data - 1]++;
                    fprintf(fp_1, "%d,%d\n", temp->data, temp->next->data);
                }
                temp = temp->next;
            }
        }
    }
    fprintf(fp, "type %d makespan %0.2lf\n", R->IVT+1, R->makespan);
    fprintf(fp, "distance %0.3lf\n", distance);
    if(fclose(fp_1) != 0){
        perror("Error closing fp_1!\n");
        exit(1);
    }

    for(int i = 0; i < G->n_customers; i++){
        if(da_access[i] == 1 && detect_dup[i] != 1){
            fprintf(fp, "fprint_results_VT detect_dup[%d] = %d\n", i, detect_dup[i]);
        }
    }

    if(fclose(fp) != 0){
        perror("Error closing fp!\n");
        exit(1);
    }

    free(vehicleRoute);

    return;
}

void fprint_data_AACONC(int ivt, int iterations, int best_iter, double foundtime, double runtime){
    char file_name[25];
    sprintf(file_name, "p%02d-%d_AACONC.data", instance_id, AACORUN);
    FILE *fp;
    if(NULL == (fp = fopen(file_name, "w"))){
        perror("Error opening data file!\n");
        exit(1);
    }

    fprintf(fp, "Solution found in: %.2lf seconds in iteration %d\n\nTotal runtime: %.2lf seconds\nTotal iterations: %d",foundtime, best_iter, runtime, iterations);
    
    if(fclose(fp) != 0){
        perror("Error closing data file!\n");
        exit(1);
    }

    
    return;
}

void fprint_data_total(double runtime){
    char file_name[25];
    sprintf(file_name, "p%02d-%d_v%d.data", instance_id, AACORUN, version_result);
    FILE *fp;
    if(NULL == (fp = fopen(file_name, "w"))){
        perror("Error opening data file!\n");
        exit(1);
    }

    fprintf(fp, "Solution found in: %.2lf seconds", runtime);

    if(fclose(fp) != 0){
        perror("Error closing data file!\n");
        exit(1);
    }

    version_result++;

    return;
}

//heuristic misc functions
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
                    if(d < d_min - epsilon){
                        d_min = d;
                        depID = G->a_depots[i].id;
                    }
                }
            }
        }
    }
    if(depID == -1){
        printf("find_closest_depot returns -1!\n");
        exit(1);
    }
    return depID;
}

adj_node *create_adj_list(int nodeID, SON *G, int *arr, int limit){
    adj_node *head = NULL;
    for(int i = 0; i < limit; i++){
        if(arr[i] != nodeID){
            double d = G->d_matrix[nodeID - 1][arr[i] - 1];
            adj_add_node(&head, arr[i], d);
        }
    }
    return head;
}

void initialization1(SON *G, asolution *R, VType *VT, int **da_access, 
                     adj_node **adj_matrix, int *cluster, int n, int depotID){

    int v_free[G->n_customers];
    for(int i = 0; i < G->n_customers; i++)
        v_free[i] = 0;

    //Get customers that are not in Truck route
    for(int i = 0; i < n; i++){
        if(da_access[0][cluster[i] - 1] == 0)    //Not truck accessible
            v_free[cluster[i] - 1] = 1;
    }

    //All customers are in Truck NN
    if(!not_empty(v_free, G->n_customers)){
        return;
    }

    int idep = depotID - G->n_customers - 1;
    if(G->a_depots[idep].n_VT[1] == 0 && G->a_depots[idep].n_VT[2] == 0 && not_empty(v_free, G->n_customers)){
        printf("init1 error: Not enough vehicles in depot!");
        exit(1);
    }

    //Initialization 1 - 
    //Epilegei ton kontinotero pelati kathe fora 
    //an epileksei Motorbike ftiaxnei route toso oso to capacity tou me nearest neighbour
    bool sw = false;
    while(not_empty(v_free, G->n_customers)){
        if((R->a_VT[2].a_depots[idep].makespan < R->a_VT[1].a_depots[idep].makespan - epsilon) && 
            (G->a_depots[idep].n_VT[2] != 0) && 
            sw == false)
        {
            adj_node *p = adj_matrix[depotID - 1];
            while(p && (v_free[p->id-1] == 0 || da_access[2][p->id - 1] == 0)){
                p = p->next;
            }
            if(!p){
                //printf("Couldn't find customer for Drone!\n");
                sw += true;
                continue;
            }
            push(&R->a_VT[2].a_depots[idep].routelist, depotID);
            push(&R->a_VT[2].a_depots[idep].routelist, p->id);
            push(&R->a_VT[2].a_depots[idep].routelist, depotID);
            R->a_VT[2].a_depots[idep].makespan = 
                get_makespan_depot_VT(G, R->a_VT[2].a_depots[idep].routelist, G->a_depots[idep].n_VT[2], VT[2].speed);
            v_free[p->id - 1] = 0;
        }else{
            node *route_best = NULL;
            int cap = VT[1].capacity, load = 0;
            push(&R->a_VT[1].a_depots[idep].routelist, depotID);
            adj_node *p = adj_matrix[depotID - 1];
            while(p && load < cap){
                while(p && (v_free[p->id - 1] == 0 || da_access[1][p->id - 1] == 0)){
                    p = p->next;
                }
                if(!p) break;

                push(&R->a_VT[1].a_depots[idep].routelist, p->id);
                v_free[p->id - 1] = 0;
                load++;
                p = adj_matrix[p->id - 1];

            }
            push(&R->a_VT[1].a_depots[idep].routelist, depotID);

            if(listLength(R->a_VT[1].a_depots[idep].routelist) == 2){
                printf("Error in init1 - no customer was added to Motorbike route\n");
                exit(1);
            }

            R->a_VT[1].a_depots[idep].makespan = 
                get_makespan_depot_VT(G, R->a_VT[1].a_depots[idep].routelist, G->a_depots[idep].n_VT[1], VT[1].speed);
        }
    }


    return;
}

