#include <stdio.h>
#include <stdlib.h>
#include "header_files/listFunctions.h"
#include "header_files/structs.h"

void adj_add_node(adj_node **head_ref, int id, double distance){
    adj_node *new_node  = malloc(sizeof *new_node);
    new_node->id        = id;
    new_node->distance  = distance;
    new_node->next      = NULL;
    if(!(*head_ref)){
        new_node->next = NULL;
        *head_ref = new_node;
        return;
    }
    //push
    if(distance < (*head_ref)->distance){
        new_node->next = *head_ref;
        *head_ref = new_node;
        return;
    }
    adj_node *t = (*head_ref), *temp = NULL;
    while(t && t->next){
        if(distance < t->next->distance - epsilon && distance >= t->distance){
            temp = t->next;
            t->next = new_node;
            new_node->next = temp;
            return;
        }
        t = t->next;
    }
    //added to the end of the list
    t->next = new_node;
    return;
}

void deleteAdjLists(adj_node **head, int n){
    for(int i = 0; i < n; i++){
        adj_node *current = head[i];
        adj_node *next;
        while(current != NULL){
            next = current->next;
            free(current);
            current = next;
        }
        head[i] = NULL; 
    }
    return;
}

void push(node **head_ref, int new_data){
    node *new_node = malloc(sizeof *new_node);
    new_node->data = new_data;
    new_node->next = *head_ref;
    *head_ref      = new_node;
    return;
}

void insertAfterNode(node *prev_node, int new_data){
    if (prev_node == NULL) {
        printf("the given previous node cannot be NULL");
        return;
    }
    node *new_node = malloc(sizeof *new_node);
    new_node->data = new_data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
    return;
}


void append(node **head_ref, int new_data){
    node *new_node = malloc(sizeof *new_node);
    node *last = *head_ref;
    new_node->data  = new_data;
    new_node->next = NULL;
    if (*head_ref == NULL){
       *head_ref = new_node;
       return;
    }  
    while (last->next != NULL){
        last = last->next;
    }
    last->next = new_node;
    return;    
}

// function to insert a Node at required position 
void insertPos(node **current, int pos, int data){ 
    while(pos--){ 
        if(pos == 0){ 
            // adding Node at required position 
            node *new_node = malloc(sizeof *new_node); 
            // Making the new Node to point to  
            // the old Node at the same position 
            new_node->data = data;
            new_node->next = *current; 

            // Changing the pointer of the Node previous  
            // to the old Node to point to the new Node 
            *current = new_node; 
        }else{
            // Assign double pointer variable to point to the  
            // pointer pointing to the address of next Node  
            current = &((*current)->next); 
        }
    } 
} 

void deleteInPosition(node **head, int position){
    node* temp;
    node* prev;
    temp = *head;
    prev = *head;
    for(int i = 0; i < position; i++){
        if(i == 0 && position == 1){
            *head = (*head)->next;
            free(temp);
        }
        else{
            if(i == position - 1 && temp != NULL){
                prev->next = temp->next;
                free(temp);
            }else{
                prev = temp;
                // Position was greater than number of nodes in the list
                if (prev == NULL)
                    break;
                temp = temp->next;
            }
        }
    }
}

void deleteOnKey(node **head_ref, int key){

    node *temp = *head_ref, *prev;
    // If head node itself holds the key to be deleted
    if (temp != NULL && temp->data == key) {
        *head_ref = temp->next; // Changed head
        free(temp);
        return;
    }
    while (temp != NULL && temp->data != key) {
        prev = temp;
        temp = temp->next;
    }
    // If key was not present in linked list
    if (temp == NULL){return;}

    // Delete node and return
    prev->next = temp->next;
    free(temp);
    return;
}

int getNth(node *head, int index){
    node *current = head;
    int count = 0;
    while (current != NULL) {
        if (count == index)
            return (current->data);
        count++;
        current = current->next;
    }
    return -1;
}

//index starts from 0 | position starts from 1
node *getNth_node(node *head, int index){
    node *current = head;
    int count = 0;
    while(current){
        if(count == index)
            return (current);
        count++;
        current = current->next;
    }
    return NULL;
}

void deleteList(node **head_ref){
   node *current = *head_ref;
   node *next;
   while(current != NULL){
       next = current->next;
       free(current);
       current = next;
   }
   *head_ref = NULL; 
   return;
}

void printList(node *p){
  while (p != NULL){
     printf(" %d ", p->data);
     p = p->next;
  }
  printf("\n");
  return;
}

void fprintList(node *p, FILE *fp){
    fprintf(fp, "%6d ", p->data);
    p = p->next;
    while (p != NULL){
        fprintf(fp, "%d ", p->data);
        p = p->next;
    }
    fprintf(fp, "\n");
    return;
}

node *copyList(node *head){
    if(head == NULL){
        return NULL;
    }else{
        node *new_node = malloc(sizeof *new_node);
        new_node->data = head->data;
        new_node->next = copyList(head->next);
        return new_node;
    }
}

void linkToTail(node **head, node *beingAdded){
    if(beingAdded == NULL)
        return;
    node *temp = *head;
    if(!temp){
        *head = beingAdded;
        return;
    }
    node *tail = NULL;
    while(temp){
        tail = temp;
        temp = temp->next;
    }
    tail->next = beingAdded;

    return;
}

int listLength(node *head){
    int len = 0;
    for ( ; head != NULL; head = head->next, ++len );
    return len;
}
