#ifndef AACO_misc_functions
#define AACO_misc_functions

#include <stdbool.h>
#include "structs.h"

void offload_to_motorbikes(SON *G, VType *VT, asolution *R_best, vt_solution *Rt_best, int **da_access, int m_capacity);

double calculate_pheromone_sum(int ilast, node *v_cand, double *phMatrix, int ivt, int n_nodes);

//quickSort functions
void quickSort(double *arr, int low, int high);

int partition(double *arr, int low, int high);

void swap(double *p1, double *p2);
//

double get_makespan_depot_VT(SON *G, node *routelist, int n_vehicles, int ivt);

double get_makespan_VT(SON *G, vt_solution *R);

double get_total_makespan(asolution *R, int differentTypes);

double calculate_h(SON *G, node* v_cand, int ilast);

double calculate_angle(int x, int y);

void find_polar_coordinates(SON *G, int vi, polar *polarArray);

void assign_vertices_to_sectors(SON *G, int vi, polar *polarArray, sector *sec, int n_sect);

bool not_empty_clusters(int *array, int length, int *da_access);

bool not_empty(int *array, int length);

bool search_cluster_VT(int *K_v, int icluster, int *v_free, int n_size);

int find_closest_free_vertex_in_sector(polar *polarArray, node *vertexList);

void find_free_in_clusterk_VT(node **head, int *v_free, int *K_v, int icluster, int n_size);

int find_closest_free_vertex(int *v_free, polar *polarArray, int numOfCustomers);

int errCheck(int value, int lower, int upper);

void new_best(asolution *new_r, asolution *r, SON *G);

void new_best_vt(vt_solution *new_r, vt_solution *r, SON *G);

int store_edge_count(vt_solution *Ra, SON *G, int *edge_matrix);

void update_pheromone(SON *G, vt_solution *Rt, vt_solution *Rt_best, asolution *R, asolution *R_best, double *phMatrix, double T_update, double d);

double evaporate_pheromones(SON *G, double *phMatrix, int *edge_matrix, int edge_sum, 
                            int n_ants, double p_min, double p_max);

void fprint_results(asolution *R, SON *G, VType *VT, int **da_access);

void fprint_results_VT(vt_solution *R, SON *G, VType *VT, int *da_access);

void fprint_data_hybrid(VType *VT, int iterations, int best_iter, double foundtime, double runtime, double term_condition, int ND);

#endif
