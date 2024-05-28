#ifndef heuristic_main_functions
#define heuristic_main_functions

#include "structs.h"

ClusterData DBSCAN_plus_kmeans(SON *G);

ClusterData k_means(SON *G);

void heuristic(SON *G, VType *VT, int **da_access, asolution *Rz, plotClusters *pl);

ClusterData createClusters(SON *G, int **da_access);

double NearestNeighbour(adj_node **adj_matrix, SON *G, asolution *R, int *da_access, int *cluster, int n, int depotID, int truck_capacity);

void initialization1(SON *G, asolution *R, VType *VT, int **da_access, 
                     adj_node **adj_matrix, int *cluster, int n, int depotID);

#endif
