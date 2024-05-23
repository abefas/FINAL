
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header_files/AACO_main_functions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/listFunctions.h"
#include "header_files/local_opt.h"

extern time_t begin_total;

void heuristic_lo(SON *G, VType *VT, asolution *R, int **da_access, int *remaining){

    //Generate clusters - takes into consideration depots' vehicle availability
    //e.g. if closest depot has only drones but customer cannot be accessed by drones then customer 
    //goes into the second closest depot's cluster
    ClusterData cd = createClusters_prox(G, da_access, remaining);

    //Initialization1 here for Motorbike and Drones
    adj_node **adj_matrix;
    if(NULL == (adj_matrix = malloc(sizeof **adj_matrix * G->n_nodes))){
        printf("Error mallocing adj_matrix!\n");
        exit(1);
    }
    for(int i = 0; i < G->n_nodes; i++){
        adj_matrix[i] = NULL;
    }

    // heuristic for each depot and its clustered customers
    for(int IDEPOT = 0; IDEPOT < G->n_depots; IDEPOT++){

        //Else adjacency matrix with customers in current cluster
        for(int i = 0; i < cd.limit[IDEPOT]; i++){
            adj_matrix[cd.cluster[IDEPOT][i] - 1] = create_adj_list(cd.cluster[IDEPOT][i], G, cd.cluster[IDEPOT], cd.limit[IDEPOT]);
        }
        //Depot adjacency to customers
        int depotID = G->n_customers + IDEPOT + 1;
        adj_matrix[depotID - 1] = create_adj_list(depotID, G, cd.cluster[IDEPOT], cd.limit[IDEPOT]);

        //Assign leftover customers to drone and motorbike
        initialization1(G, R, VT, da_access, adj_matrix, cd.cluster[IDEPOT], cd.limit[IDEPOT], depotID);

        //Continue to next depot
        deleteAdjLists(adj_matrix, G->n_nodes);
    }
    free(adj_matrix);



    //R->move unnecessary nodes(depot to depot) and ready for final local opt
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        for(int idep = 0; idep < G->n_depots; idep++){
           remove_duplicate_nodes(&R->a_VT[ivt].a_depots[idep].routelist);
        }
    }


    //Get Vehicle types' makespans - were not needed/used until now 
    //And resulting total makespan
    R->total_makespan = 0.0;
    for(int ivt = 0; ivt < G->n_differentTypes; ivt++){
        R->a_VT[ivt].makespan = get_makespan_VT(G, &R->a_VT[ivt]);

        if(R->total_makespan < R->a_VT[ivt].makespan)
            R->total_makespan = R->a_VT[ivt].makespan;
    }



    R->total_makespan = local_opt_full(R, G, da_access, VT);


    time_t finish_total = time(NULL);
    double runtime_total = difftime(finish_total, begin_total);

    fprint_results(R, G, VT, da_access);
    fprint_data_total(runtime_total);

    for(int dep = 0; dep < G->n_depots; dep++){
        free(cd.cluster[dep]);
    }
    free(cd.cluster);
    free(cd.limit);

    return;
}

