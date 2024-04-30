#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_TYPES 3

#define DRONE_CHANCE 85
#define TRUCK_CHANCE 90

#define MOTORBIKE_CAPACITY 3
#define DRONE_CAPACITY 1

struct vertex{
    int nodeID;
    float x;
    float y;
};

int main(int argc, char **argv){

    int instance_id;
    if(argc != 2){
        printf("Usage: <program> <instance_id>\n");
        exit(1);
    }else{
        if( sscanf(argv[1], "%02d", &instance_id) != 1) {
            printf("Invalid instance ID\n");
            exit(EXIT_FAILURE);
        }
    }


    char file_name_new[30], cordeau_file[50];
    FILE *f, *fn;
    
    //Get cordeau file
    sprintf(cordeau_file, "../../Cordeau_dataset_original/p%02d.txt", instance_id);
    if(NULL == (f = fopen(cordeau_file, "r"))){
        perror("Error opening f\n");
        exit(1);
    }

    //Create and open cordeau-HVRP file
    sprintf(file_name_new, "p%02d.MDmfcmTSP", instance_id);
    if(NULL == (fn = fopen(file_name_new, "w"))){
        perror("Error opening fn\n");
        exit(1);
    }

    int customers, depots, TRUCK_CAPACITY;
    char line[100];

    fscanf(f, "%*d %*d %d %d", &customers, &depots);

    //Skip Cordeau depots' capacites and route max length
    for(int flines = 0; flines < depots; flines++){
        fgets(line, sizeof line, f);
        sscanf(line, "%*d %d", &TRUCK_CAPACITY);
    }
    TRUCK_CAPACITY /= 10;

    fprintf(fn, "%d %d %d\n", customers, depots, N_TYPES);

    fprintf(fn, "%d %d %d\n", TRUCK_CAPACITY, MOTORBIKE_CAPACITY, DRONE_CAPACITY);

    double v_Truck = 15.0, v_Motorbike = 25.0, v_Drone = 30.0;
    fprintf(fn, "%0.1lf %0.1lf %0.1lf\n", v_Truck, v_Motorbike, v_Drone);


    struct vertex *arr = malloc(sizeof *arr * (customers+depots));

    //Get demand of each customer in Cordeau instance
    int i = 0;
    while(fgets(line, sizeof line, f) != NULL){
        fscanf(f, "%*d %f %f", &arr[i].x, &arr[i].y);
        i++;
    }

    time_t t;
    srand((unsigned) time(&t));

    for(int i = 0; i < customers; i++){
        char acc[3] = {'\0'};

        if(random() % 100 < TRUCK_CHANCE){
            acc[0] = '1';
        }

        acc[1] = '2';       //Motorbike can server every customer

        if(random() % 100 < DRONE_CHANCE){
            acc[2] = '3';
        }


        char ac[3] = {'\0'};
        int p = 0;
        for(int j = 0; j < 3; j++){
            if(acc[j] != '\0'){
                ac[p] = acc[j];
                p++;
            }
        }
        int access = atoi(ac);
        int nodeID = i+1;
        fprintf(fn, "%2d\t %8.2f\t %8.2f\t %2d %4d\n", nodeID, arr[i].x/2*100, arr[i].y/2*100, 1, access);
    }

    for(int i = customers; i < (customers+depots); i++){
        int depotID = i + 1;
        fprintf(fn, "%2d\t %8.2f\t %8.2f\t %2d %2d %2d\n", depotID, arr[i].x/2*100, arr[i].y/2*100, 1, 1, 1);
    }

    if(fclose(fn) != 0) perror("Error closing fn\n");
    if(fclose(f) != 0) perror("Error closing f");
    return 0;
}
