#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "timer.h"

#define LONG long long
#define MAX_THREADS 12
#define USAGE() {                                                               \
    fprintf(stderr, "Usage: %s <threads> <n> <log file, optional> \n", argv[0]);\
    fprintf(stderr, "       1 <= `threads` <= %d\n", MAX_THREADS);              \
    fprintf(stderr, "       `n` >= 1 and divisible by `threads`\n");            \
    exit(EXIT_FAILURE);                                                         \
}

#define ASSERT(call)             \
    if (call < 0) {              \
        perror(#call " failed"); \
        exit(EXIT_FAILURE);      \
    }                            \


double serial_pi(LONG n) {
    LONG arrows = 0;

    double range = (1.0 - (-1.0)); 
    double div = RAND_MAX / range;
    unsigned int mystate = time(NULL);

    for (LONG i = 0; i < n; i++) {
        double x = -1.0 + (rand_r(&mystate) / div);
        double y = -1.0 + (rand_r(&mystate) / div);

        if (x * x + y * y <= 1)
            arrows++;
    }

    return 4 * arrows / (double) n;
}


double threaded_pi(size_t threads, LONG n) {
    LONG arrows = 0;
    
	#pragma omp parallel num_threads(threads) \
    reduction(+: arrows)
    {
        double range = (1.0 - (-1.0)); 
        double div = RAND_MAX / range;
        unsigned int mystate = time(NULL) - omp_get_thread_num();

        #pragma omp for
        for (LONG i = 0; i < n; i++) {
            double x = -1.0 + (rand_r(&mystate) / div);
            double y = -1.0 + (rand_r(&mystate) / div);

            if (x * x + y * y <= 1)
                arrows++;
        }
    }

    return 4 * arrows / (double) n;
}

int main(int argc, char* argv[]) {
    if (argc < 3)
        USAGE()
    
    size_t threads = atol(argv[1]);
    LONG n = atoll(argv[2]);

    if (threads > MAX_THREADS || n == 0)
        USAGE()

    fprintf(stdout, "+-----------+------------+---------------+\n");
    fprintf(stdout, "|  Threads  |     pi     | elapsed (sec) |\n");
    fprintf(stdout, "|-----------+------------+---------------|\n");

    double start, finish, pi;

    GET_TIME(start);
    pi = threads == 1 ? serial_pi(n) : threaded_pi(threads, n);
    GET_TIME(finish);

    double threaded_time = finish - start;
    fprintf(stdout, "|    %2ld     | %.8f | %13.8f |\n", threads, pi, threaded_time);

    fprintf(stdout, "+-----------+------------+---------------+\n");

    if (argc == 4) {
        FILE* file = fopen(argv[3], "a");

        fprintf(file, "%lld,%ld,%f\n", n, threads, threaded_time);

        fclose(file);
    }

    return 0;
}