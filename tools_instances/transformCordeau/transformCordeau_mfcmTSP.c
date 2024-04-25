#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_TYPES 3

#define DEMAND_1 85

#define DRONE_CHANCE 85
#define TRUCK_CHANCE 80

#define TRUCK_CAPACITY 8
#define MOTORBIKE_CAPACITY 3
#define DRONE_CAPACITY 1

struct vertex{
    int nodeID;
    int x;
    int y;
};

int main(){

    FILE *f = fopen("p10.txt", "r");
    FILE *fn = fopen("p10_new.txt", "w");

    int garbage, customers, depots;

    fscanf(f, "%d %d %d %d", &garbage, &garbage, &customers, &depots);

    fprintf(fn, "%d %d %d\n", customers, depots, N_TYPES);

    fprintf(fn, "%d %d %d\n", TRUCK_CAPACITY, MOTORBIKE_CAPACITY, DRONE_CAPACITY);

    double s1 = 5.0, s2 = 5.0, s3 = 5.0;
    fprintf(fn, "%0.1lf %0.1lf %0.1lf\n", s1, s2, s3);

    struct vertex *arr = malloc(sizeof *arr * (customers+depots));

    //Skip Cordeau depots' capacites
    for(int flines = 0; flines < depots; flines++){
        fscanf(f, "%d %d", &garbage, &garbage);
    }
    //Get vertices
    for(int flines = 0; flines < customers; flines++){
        fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d", &arr[flines].nodeID, &arr[flines].x, &arr[flines].y,
               &garbage,&garbage,&garbage,&garbage,&garbage,&garbage,&garbage,&garbage);
    }
    for(int flines = customers; flines < (customers+depots); flines++){
        fscanf(f, "%d %d %d %d %d %d %d", &arr[flines].nodeID, &arr[flines].x, &arr[flines].y,
               &garbage,&garbage,&garbage,&garbage);
    }

    time_t t;
    srand((unsigned) time(&t));

    for(int i = 0; i < customers; i++){
        int demand = 1;
        char acc[3] = {'\0'};

        if(rand() % 100 < TRUCK_CHANCE) acc[0] = '1';

        acc[1] = '2';

        if(rand() % 100 < DRONE_CHANCE) acc[2] = '3';


        char ac[3] = {'\0'};
        int p = 0;
        for(int j = 0; j < 3; j++){
            if(acc[j] != '\0'){
                ac[p] = acc[j];
                p++;
            }
        }
        int access = atoi(ac);
        fprintf(fn, "%d %d %d %d %d\n", arr[i].nodeID, arr[i].x, arr[i].y, demand, access);
    }

    for(int i = customers; i < (customers+depots); i++){
        fprintf(fn, "%d %d %d %d %d %d\n", arr[i].nodeID, arr[i].x, arr[i].y, 1, 1, 1);
    }

    free(arr);
    fclose(fn);
    fclose(f);
    return 0;
}
