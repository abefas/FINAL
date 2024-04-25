#ifndef AACO_misc_functions
#define AACO_misc_functions

#include <stdbool.h>
#include "structs.h"

double calculate_pheromone_sum(int idep, int ivt, int ilast, node *v_cand, double *phMatrix, int n_nodes, int dt);

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

int find_closest_free_vertex(int *v_free, polar *polarArray, int numOfCustomers, int *da_access);

void find_free_in_clusterk_VT(node **head, int *v_free, int *K_v, int icluster, int n_size);

int errCheck(int value, int lower, int upper);

void new_best(asolution *new_r, asolution *r, SON *G);

int store_edge_count(asolution *Ra, SON *G, int *edge_matrix, int *edges);

void update_pheromones(SON *G, double *phMatrix, asolution *R, asolution *R_best, double tupdate, double d);

double evaporate_pheromones(SON *G, int *edge_matrix, double *phMatrix, int edge_sum, int n_ants, int *edges, double p_min, double p_max);

void fprint_results(asolution *R, SON *G, VType *VT);

void fprint_data(int iterations, int best_iter, double foundtime, double runtime);

#endif
