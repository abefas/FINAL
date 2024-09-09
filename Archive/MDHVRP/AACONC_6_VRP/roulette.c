#include <stdio.h>
#include <stdlib.h>
#include "header_files/roulette.h"
#include "header_files/structs.h"

void push_rlist(rlist **head_ref, int new_id, double prob){
    rlist *new_node = malloc(sizeof *new_node);
    new_node->id = new_id;
    new_node->prob = prob;
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
    return;
}

void append_rlist(rlist **head_ref, int new_id, double prob){
    rlist *new_node = malloc(sizeof *new_node);
    rlist *last = *head_ref;
    new_node->id  = new_id;
    new_node->prob = prob;
    new_node->next = NULL;
    if(*head_ref == NULL){
       *head_ref = new_node;
       return;
    }  
    while(last->next != NULL)
        last = last->next;
    
    last->next = new_node;
    return;    
}

void delete_rlist(rlist **head_ref){
    rlist *current = *head_ref;
    rlist *next;
    while (current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
    *head_ref = NULL; 
    return;
}

int roulette(double* p, int pLen, double p_sum){
    double sum = 0.0;
    double r = ((double)random()/RAND_MAX) * p_sum;
    for (int i = 0; i < pLen; i++) {
        sum += p[i];
        if (sum >= r) {
            return i;
        }
    }
    return -1;
}

int roulette_list(rlist *head, double p_max){
    if(!head) return -1;
    if(!head->next) return head->id;
    double sum = 0.0;
    double r = ((double)random()/RAND_MAX) * p_max;
    rlist *temp = head;
    while(temp){
        sum += temp->prob;
        if(r - epsilon <= sum){
            return temp->id;
        }
        temp = temp->next;
    }
    printf("p_max = %0.3lf\n", p_max);
    printf("r = %0.3lf\n", r);
    return -1;
}
