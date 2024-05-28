#ifndef heuristic_misc_functions
#define heuristic_misc_functions

#include <stdbool.h>
#include "structs.h"

void save_clustering_to_csv(Point *points, Point *centroids, depot *depots, int num_points, int k);

void plot_clusters(Point *points, Point *centroids, depot *depots, int num_points, int k);

double get_makespan_depot_VT(SON *G, node *routelist, int n_vehicles, double speed);

double get_makespan_VT(SON *G, vt_solution *R);

double get_total_makespan(asolution *R, int differentTypes);

int find_closest_depot(int customerID, SON *G, int **da_access);

double get_time_VT(node *route, SON *G, double speed);

adj_node *create_adj_list(int i, SON *G, int *arr, int n);

bool not_empty(int *array, int length);

void fprint_results(asolution *R, SON *G, VType *VT);

void fprint_data(double runtime);

int addToRoute(SON *G, node **route, int customerID, int capacity, double speed);

//quickSort functions
void quickSort(double *arr, int low, int high);

int partition(double *arr, int low, int high);

void swap(double *p1, double *p2);
//


#endif
