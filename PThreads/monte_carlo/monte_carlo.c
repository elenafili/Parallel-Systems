#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include "timer.h"

#define LONG long long
#define MAX_THREADS 1024
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


double approx_percentage(LONG n) {
    LONG arrows_hit = 0;

    double range = (1.0 - (-1.0)); 
    double div = RAND_MAX / range;
    unsigned int mystate = time(NULL);

    for (LONG i = 0; i < n; i++) {
        double x = -1.0 + (rand_r(&mystate) / div);
        double y = -1.0 + (rand_r(&mystate) / div);

        if (x * x + y * y <= 1)
            arrows_hit++;
    }

    return arrows_hit;
}

// Shared Variables between threads
LONG arrows = 0;
pthread_mutex_t mutex;

void* worker(void* args) {
    LONG arrows_hit = approx_percentage(*(LONG*)args);

    ASSERT(pthread_mutex_lock(&mutex));
    arrows += arrows_hit;
    ASSERT(pthread_mutex_unlock(&mutex));

    return NULL;
}

double threaded_pi(size_t threads, LONG n) {

    LONG n_thread = n / threads;

    pthread_t* tids = malloc(sizeof(*tids) * threads); 
    ASSERT(pthread_mutex_init(&mutex, NULL));

    for (size_t i = 0; i < threads; i++)  
        ASSERT(pthread_create(&tids[i], NULL, worker, &n_thread)); 

    for (size_t i = 0; i < threads; i++)
        ASSERT(pthread_join(tids[i], NULL));

    ASSERT(pthread_mutex_destroy(&mutex));
    free(tids);

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
    pi = threaded_pi(threads, n);
    GET_TIME(finish);

    double threaded_time = finish - start;
    fprintf(stdout, "|     %ld     | %.8f | %13.8f |\n", threads, pi, threaded_time);

    fprintf(stdout, "+-----------+------------+---------------+\n");

    if (argc == 4) {
        FILE* file = fopen(argv[3], "a");

        fprintf(file, "%lld,%ld,%f\n", n, threads, threaded_time);

        fclose(file);
    }

    return 0;
}