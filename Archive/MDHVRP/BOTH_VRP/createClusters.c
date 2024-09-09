#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/AACO_misc_functions.h"
#include "header_files/structs.h"
                                    
void createClusters(int *K, SON *G, int vi, int n_size, int n_sect, int *da_access){

    int *v_free = malloc(sizeof *v_free * G->n_customers);
    if(!v_free){ perror("Couldn't malloc v_free at createClusters.c\n"); exit(1); }
    for(int i = 0; i < G->n_customers; i++){
        v_free[i] = 1;
        if(vi == i){
            v_free[i] = -1; //Cluster Vertex
        }
    }

    polar *polarArray = malloc(sizeof *polarArray * G->n_customers);
    find_polar_coordinates(G, vi, polarArray);

    sector *sec = malloc(sizeof *sec * n_sect);
    assign_vertices_to_sectors(G, vi, polarArray, sec, n_sect);
        
    /* Add closest of each sector in cluster 1 */  
    int id = 0, closest;                            //id of cluster (number of cluster) starting from 0
    int K_index = 0;                                //column index for "matrix" K
    for (int j = 0; j < n_sect; j++){
        if(!sec[j].vertexList) continue;
        closest = find_closest_free_vertex_in_sector(polarArray, sec[j].vertexList, da_access);
        if(closest != -1){
            K[id * n_size + K_index] = closest; //If Sector has at least one vertex then add the vertex to Cluster number 1 (id=0)
            v_free[closest - 1] = -1;                 //change vertex as added to a cluster (not free)
            K_index++;
        }
    }


/* Continue filling cluster 1 and then the rest */ //(n_sect<n_size)
    while(not_empty_clusters(v_free, G->n_customers, da_access)){
        closest = find_closest_free_vertex(v_free, polarArray, G->n_customers, da_access);
        if(closest != -1){
            if(K_index == n_size){                  //Cluster full
                id++;                               //Start adding to next cluster id (change row in "matrix")
                K_index = 0;
            }
            K[id * n_size + K_index] = closest;
            v_free[closest - 1] = -1;                 //mark node as visited
            K_index++;    
        }
    }

    /* Free memory */
    for(int i = 0; i < n_sect; i++) deleteList(&sec[i].vertexList);  
    free(sec);
    free(polarArray);
    free(v_free);
    
    return; 
}

//No sectors, just closest vertices
void createClusters_drone(int *K, SON *G, int vi, int n_size, int *da_access){

    //vi = depot vertex
    adj_node *adj_list = NULL;
    for(int i = 0; i < G->n_customers; i++){
        if(da_access[i] == 1){
            double d = G->d_matrix[vi][i];
            adj_add_node(&adj_list, i+1, d);
        }
    }

    int K_index = 0, id = 0;
    adj_node *p = adj_list;
    while(p){
        if(K_index == n_size){
            id++;
            K_index = 0;
        }
        K[id*n_size + K_index] = p->id;
        K_index++;
        p = p->next;
    }

    deleteAdjLists(&adj_list, 1);
    return;
}
