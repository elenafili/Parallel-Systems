#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <mpi.h>
#include "timer.h"

#define ROOT 0
#define LONG long long

int main(int argc, char* argv[]) {

    MPI_Init(NULL, NULL);

    double start, finish, elapsed;
    int comm_sz, rank;
    LONG n, total_arrows;

    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == ROOT){
        fprintf(stdout, "Give number of arrow to be thrown: ");
        scanf("%lld", &n);
        
        GET_TIME(start);
    }

    MPI_Bcast(&n, 1, MPI_LONG, ROOT, MPI_COMM_WORLD);

    LONG arrows = 0;

    double range = (1.0 - (-1.0)); 
    double div = RAND_MAX / range;
    unsigned int mystate = time(NULL) ^ time(rank);

    for (LONG i = 0; i < n; i++) {
        double x = -1.0 + (rand_r(&mystate) / div);
        double y = -1.0 + (rand_r(&mystate) / div);

        if (x * x + y * y <= 1)
            arrows++;
    }

    MPI_Reduce(&arrows, &total_arrows, 1, MPI_LONG, MPI_SUM, ROOT, MPI_COMM_WORLD);

    if(rank == ROOT) {

        GET_TIME(finish);
        elapsed = finish - start;

        double pi = 4 * arrows_in_circle / (double) n;
        
        fprintf(stdout, "+-----------+------------+---------------+\n");
        fprintf(stdout, "| Processes |     pi     | elapsed (sec) |\n");
        fprintf(stdout, "|-----------+------------+---------------|\n");
        fprintf(stdout, "|    %2ld     | %.8f | %13.8f |\n", comm_sz, pi, elapsed);
        fprintf(stdout, "+-----------+------------+---------------+\n");

        
        if (argc == 2) {
            FILE* file = fopen(argv[1], "a");

            fprintf(file, "%lld,%ld,%f\n", n, comm_sz, elapsed);

            fclose(file);
        }
    } 

    MPI_Finalize();
    
    return 0;
}