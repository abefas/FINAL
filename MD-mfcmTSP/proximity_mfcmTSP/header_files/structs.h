#ifndef structs
#define structs
#include <stdbool.h>
#include "listFunctions.h"

#define epsilon 0.000001

typedef struct Customer{
    int id;
    int x;
    int y;
    int demand;
} customer;

typedef struct Combined{
    int id;
    int x;
    int y;
} combined;

typedef struct Depot{
    int id;
    int x;
    int y;
    int *n_VT;          //Stores how many vehicles of each type the depot has - isnt't used in VT.a_depots
} depot;

typedef struct VehicleType{
    int IVT;
    int n_vehicles;
    int n_customers;
    int n_depots;
    int capacity;
    double speed;
} VType;

typedef struct SetOfNodes{
    int n_customers;
    int n_depots;
    int n_nodes;
    int n_differentTypes;
    double **d_matrix;
    customer *a_customers;
    depot *a_depots;
    combined *a_combined;
} SON;

typedef struct Route{
    double makespan;
    int quantity_served;
    int depot_id;
    node *routelist;
} route;

typedef struct vt_solution{
    int IVT;
    double makespan;
    route *a_depots;
} vt_solution;

typedef struct asolution{
    double total_makespan;
    vt_solution *a_VT;
} asolution;


struct adj_list{
   int nodeID;
   float distance;
};

typedef struct closest{
    int id;
    int closestDepot;
    double distance;
}closest;

typedef struct Cluster{
    int **cluster;
    int *limit;
}ClusterData; 

#endif
