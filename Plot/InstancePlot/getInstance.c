#include <stdio.h>
#include <stdlib.h>
struct vertex{
    int nodeID;
    float x;
    float y;
};

int main(){
    FILE *fi;
    if(NULL == (fi = fopen("../../Instances/Cordeau_mfcmTSP/p11.MDmfcmTSP", "r"))){
        printf("Error opening fi\n");
        exit(1);
    }

    int garb, customers, depots, n_types;
    double fgarb;
    fscanf(fi, "%d %d %d", &customers, &depots, &n_types);
    fscanf(fi, "%d %d %d", &garb, &garb, &garb);
    fscanf(fi, "%lf %lf %lf", &fgarb, &fgarb, &fgarb);

    struct vertex *arr = malloc(sizeof *arr * (customers+depots));
    if(!arr){
        printf("Error mallocing arr\n");
        exit(1);
    }

    for(int i = 0; i < customers; i++){
        fscanf(fi, "%d %f %f %d %d", &arr[i].nodeID, &arr[i].x, &arr[i].y, &garb, &garb);
    }
    for(int i = customers; i < customers+depots; i++){
        fscanf(fi, "%d %f %f %d %d %d", &arr[i].nodeID, &arr[i].x, &arr[i].y, &garb, &garb, &garb);
    }

    FILE *fn;
    if(NULL == (fn = fopen("p11_nodes.csv", "w"))){
        printf("Error opening fi\n");
        exit(1);
    }

    fprintf(fn, "NodeID,Xposition,Yposition\n");
    for(int i = 0; i < customers+depots; i++){
        fprintf(fn, "%d,%f,%f\n", arr[i].nodeID, arr[i].x, arr[i].y);
    }

    if(EOF == fclose(fn) || EOF == fclose(fi)){
        printf("Error closing files\n");
        exit(1);
    }
    free(arr);
    return 0;
}
