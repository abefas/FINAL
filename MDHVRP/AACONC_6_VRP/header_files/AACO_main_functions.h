#ifndef AACO_main_functions
#define AACO_main_functions

#include "structs.h"

void AACONC(SON *G, VType *VT, int **da_access, int n_ants, int n_freq, int n_size, int n_sect, int n_prim, 
                    double T_update, double a_update, double p_min, double p_max, double d, double a, double b);


void antSolution(SON *G, VType *VT, int ***K, double *t, asolution *R, int **da_access, 
                    int n_size, int n_prim, double a, double b);

int selectVehicleType(asolution *Ra, VType *VT, int *v_free, SON *G, int ***K, double *phMatrix, 
                        int *launch_count, int **da_access, int n_size, int n_prim);

int selectDepot(asolution *Ra, int *v_free, SON *G, int **K, double *phMatrix, int *da_access, 
                int ivt, int n_size, int n_prim);

int selectCluster(int idepot, int ivt, int ilast, int *v_free, int *K, double *phMatrix,
                  SON *G, int n_size, int n_prim, double vspeed, double a, double b);

int selectCustomer(int idep, int ivt, int ilast, node *v_cand, double *phMatrix, SON *G, 
                    int *da_access, double a, double b);


void createClusters(int *K, SON *G, int vi, int n_size, int n_prim, int n_sect, int *da_access);

void createClusters_drone(int *K, SON *G, int vi, int n_size, int *da_access);


#endif
