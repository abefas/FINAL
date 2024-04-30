#ifndef AACO_misc_functions
#define AACO_misc_functions

#include <stdbool.h>
#include "structs.h"

double calculate_pheromone_sum(int ilast, node *v_cand, double *phMatrix, int ivt, int n_nodes);

//quickSort functions
void quickSort(double *arr, int low, int high);

int partition(double *arr, int low, int high);

void swap(double *p1, double *p2);
//

double get_makespan_depot_VT(SON *G, node *routelist, int n_vehicles, double speed);

double get_makespan_VT(SON *G, vt_solution *R);

double get_total_makespan(asolution *R, int differentTypes);

double calculate_h(SON *G, node* v_cand, int ilast);

double calculate_angle(int x, int y);

void find_polar_coordinates(SON *G, int vi, polar *polarArray);

void assign_vertices_to_sectors(SON *G, int vi, polar *polarArray, sector *sec, int n_sect);

bool not_empty_clusters(int *array, int length, int *da_access);

bool not_empty(int *array, int length);

bool search_cluster_VT(int *K_v, int icluster, int *v_free, int n_size);

int find_closest_free_vertex_in_sector(polar *polarArray, node *vertexList, int *da_access);

void find_free_in_clusterk_VT(node **head, int *v_free, int *K_v, int icluster, int n_size);

int find_closest_free_vertex(int *v_free, polar *polarArray, int numOfCustomers, int *da_access);

int errCheck(int value, int lower, int upper);

void new_best(d_solution *new_r, d_solution *r, SON *G);

int store_edge_count(d_solution *Ra, SON *G, int *edge_matrix);

void update_pheromones(SON *G, double *phMatrix, d_solution *R, d_solution *R_best, double T_update, double d);

double evaporate_pheromones(SON *G, double *phMatrix, int *edge_matrix, int *da_access, int edge_sum, 
                            int n_ants, double p_min, double p_max);

void fprint_results(asolution *R, SON *G, VType *VT, int **da_access);

void fprint_results_v3(asolution *R, SON *G, VType *VT);

void fprint_results_AACONC(d_solution *R, SON *G, VType VT, int *da_access);

void fprint_data_AACONC(int ivt, int iterations, int best_iter, double foundtime, double runtime);

void fprint_data_total(double runtime);

int find_closest_depot(int customerID, SON *G, int **da_access);

adj_node *create_adj_list(int nodeID, SON *G, int *arr, int n);

void initialization1(SON *G, asolution *R, VType *VT, int **da_access, 
                     adj_node **adj_matrix, int *cluster, int n, int depotID);

#endif
