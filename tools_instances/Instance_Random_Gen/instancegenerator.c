#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SPACE_SIZE 10001 //x000 x x000 m^2
#define N_TYPES 3

#define DEMAND_1 101
#define DRONE_CHANCE 85
#define DRONE_CAPACITY 1

#define TRUCK_CHANCE 90
#define TRUCK_CAPACITY 100

#define MOTORBIKE_CAPACITY 5

int main(int argc, char **argv){

    int instance_id;
    if(argc != 2){
        printf("Usage: <program> <instance_id>\n");
        exit(EXIT_SUCCESS);
    }else{
        if( sscanf(argv[1], "%02d", &instance_id) != 1) {
            printf("Invalid instance ID\n");
            exit(EXIT_FAILURE);
        }
    }

    char file_name[30];
    sprintf(file_name, "x%02d.MDmfcmTSP", instance_id);
    FILE *f;
    if(NULL == (f = fopen(file_name, "w"))){
        perror("Error opening file!\n");
        exit(1);
    }

    int customers, depots;
    printf("Enter number of customers: ");
    scanf("%d",&customers);
    printf("Enter number of depots: ");
    scanf("%d",&depots);

    fprintf(f, "%d %d %d\n", customers, depots, N_TYPES);

    fprintf(f, "%d %d %d\n", TRUCK_CAPACITY, MOTORBIKE_CAPACITY, DRONE_CAPACITY);

    double s1 = 15.0, s2 = 20.0, s3 = 25.0;

    fprintf(f, "%0.1lf %0.1lf %0.1lf\n", s1, s2, s3);

    time_t t;
    srand((unsigned) time(&t));

    for(int i = 0; i < customers; i++){
        float x = random() % SPACE_SIZE , y = random() % SPACE_SIZE, demand = 1;
        char acc[3] = {'\0'};
        if(random() % 100 < DEMAND_1){
            demand = 1;
        }else{
            demand = random() % TRUCK_CAPACITY + 1;
        }

        if(demand <= MOTORBIKE_CAPACITY) acc[1] = '2';                  //Motorbike is only limited by demand size
        if(demand == 1 && random() % 100 < DRONE_CHANCE){
           acc[2] = '3';
        }

        if(acc[1] == '\0' && acc[2] == '\0'){
            acc[0] = '1';
        }else{
            if(random() % 100 < TRUCK_CHANCE) acc[0] = '1';
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
        fprintf(f, "%d %f %f %d %d\n", i+1, x, y, 1, access);
    }

    for(int i = customers; i < customers+depots; i++){
        float x = random() % SPACE_SIZE, y = random() % SPACE_SIZE;
        fprintf(f, "%d %f %f %d %d %d\n", i+1, x, y, 1, 1, 1);
    }



    if(fclose(f) != 0){
        perror("Error closing file!\n");
        exit(1);
    }

    return 0;
}
