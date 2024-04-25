#ifndef AACO_main_functions
#define AACO_main_functions

#include "structs.h"

void AACONC(SON *G, VType *VT, vt_solution *Rz, int **da_access, int *remaining, int ivt, int n_ants, 
        int n_freq, int n_size, int n_sect, int n_prim, double T_update, double a_update, 
        double p_min, double p_max, double d, double a, double b);


void antSolution(SON *G, VType VT, vt_solution *Ra, int **K, double *t, int *da_access, int *remaining, 
        int n_size, int n_prim, double a, double b);


int selectDepot(vt_solution *Ra, SON *G, int *v_free, int **K, double *phMatrix, int n_size, int n_prim);


int selectCluster(SON *G, int ilast, int *v_free, int *K, 
        double *phMatrix, int idepot, int n_size, int n_prim, double vspeed, double a, double b);


int selectCustomer(SON *G, int idepot, int ilast, node *v_cand, double *phMatrix, double a, double b);


void createClusters(int *K, SON *G, int vi, int n_size, int n_sect, int *da_access);

void createClusters_drone(int *K, SON *G, int vi, int n_size, int *da_access);

//heuristic main functions
ClusterData createClusters_prox(SON *G, int **da_access, int *remaining);

void heuristic_og(SON *G, VType *VT, asolution *R, int **da_access, int *remaining);

void heuristic_v1(SON *G, VType *VT, asolution *R, int **da_access, int *remaining);

void heuristic_v2(SON *G, VType *VT, asolution *R, int **da_access, int *remaining);

void heuristic_v3(SON *G, VType *VT, asolution *R, int **da_access);

void heuristic_v4(SON *G, VType *VT, asolution *R, int **da_access);

void heuristic_v5(SON *G, VType *VT, asolution *R, int **da_access);

void heuristic_v01(SON *G, VType *VT, asolution *R, int **da_access, int *remaining);

void heuristic_v01_1(SON *G, VType *VT, asolution *R, int **da_access, int *remaining);
#endif
