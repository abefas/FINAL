#ifndef rouletteWheel
#define rouletteWheel

typedef struct listForRoulette{
    int id;
    double prob;
    struct listForRoulette *next;
} rlist;

int roulette(double* p, int pLen, double p_sum);

int roulette_list(rlist *head, double p_max);

void push_rlist(rlist **head_ref, int id, double prob);

void append_rlist(rlist **head_ref, int new_id, double prob);

void delete_rlist(rlist **head);

#endif
