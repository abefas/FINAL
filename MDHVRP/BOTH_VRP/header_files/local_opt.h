#ifndef local_opt
#define local_opt 
#include "structs.h"

double local_opt_full(asolution *R, SON *G, int **da_access, VType *VT);

double depot_VT_optimization(asolution *R, SON *G, VType *VT, int **da_access, int n_max);

double k_optimization2(route *r, SON *G, VType VT, int n_max);

double mutual_k_optimization(vt_solution *R, SON *G, VType VT, int condition, int n_max);

double mutual_drone(vt_solution *R, SON *G, VType VT);

void remove_duplicate_nodes(node **list);

int check_route_feasibility(node *route, SON *G, int capacity);

#endif
