#ifndef heuristic_main_functions
#define heuristic_main_functions

#include "structs.h"

void heuristic_v1(SON *G, VType *VT, int **da_access);

void heuristic_v1_2(SON *G, VType *VT, int **da_access);

void heuristic_prox(SON *G, VType *VT, int **da_access);

void heuristic_local_opt(SON *G, VType *VT, int **da_access);

void heuristic_prox_2(SON *G, VType *VT, int **da_access);

void heuristic_prox_standalone(SON *G, VType *VT, int **da_access);

void heuristic_prox_standalone_2(SON *G, VType *VT, int **da_access);

ClusterData createClusters(SON *G, int **da_access);

double NearestNeighbour(adj_node **adj_matrix, SON *G, asolution *R, int *da_access, int *cluster, int n, int depotID, int truck_capacity);

void initialization1(SON *G, asolution *R, VType *VT, int **da_access, 
                     adj_node **adj_matrix, int *cluster, int n, int depotID);

#endif
