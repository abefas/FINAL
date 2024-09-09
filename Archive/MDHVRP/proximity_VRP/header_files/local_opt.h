#ifndef local_opt
#define local_opt 
#include "structs.h"

double k_optimization2(route *r, SON *G, VType VT, int n_max);

double mutual_k_optimization(vt_solution *R, SON *G, VType VT, int condition, int n_max);

double mutual_drone(vt_solution *R, SON *G, VType VT);

void remove_duplicate_nodes(node **list);

int check_route_feasibility(node *route, SON *G, int capacity);

#endif
