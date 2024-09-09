#ifndef listFunctions
#define listFunctions
#include <stdio.h>

typedef struct Node{
    int data;
    struct Node *next;
} node;

typedef struct adj_node{
    int id;
    double distance;
    struct adj_node *next;
}adj_node;


void adj_push(adj_node **head_ref, int id, double distance);

void adj_add_node(adj_node **head, int id, double distance);

void printAdjList(adj_node *head);

void deleteAdjLists(adj_node **head, int n);

void insertPos(node **current, int pos, int data);

void push(node **head_ref, int new_data);

void insertAfterNode(node *prev_node, int new_data);

void append(node **head_ref, int new_data);

void deleteInPosition(node **head, int position);

void deleteOnKey(node **head_ref, int key);

int getNth(node *head, int index);

node *getNth_node(node *head, int index);

void deleteList(node **head_ref);

void printList(node *p);

void fprintList(node *p, FILE *fp);

node *copyList(node *head);

void linkToTail(node **head, node *beingAdded);

int listLength(node *head);

#endif
