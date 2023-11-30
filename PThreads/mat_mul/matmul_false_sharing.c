#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
#include "my_rand.h"

#define USAGE() {                                                       \
    fprintf(stderr, "Usage: %s <m> <n> <p> <threads>\n", argv[0]);      \
    fprintf(stderr, "       m, n, p > 0 and threads >= 1\n");           \
    fprintf(stderr, "       m should be divisible by threads >= 1\n");  \
    exit(EXIT_FAILURE);                                                 \
}

#define ASSERT(call)             \
    if (call < 0) {              \
        perror(#call " failed"); \
        exit(EXIT_FAILURE);      \
    }                            \

double* A;
double* B;
double* C;


void rand_matrix(double* mat, const size_t m) {
    unsigned int seed = time(NULL);
    for (size_t i = 0; i < m; i++) 
        mat[i] = my_drand(&seed);
} 

void print_matrix(double* mat, const size_t m, const size_t n) {
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++)
            printf("%6.4f ", mat[i * m + j]);
        printf("\n");
    }
} 

void matrices_init(const size_t m, const size_t n, const size_t p) {
    A = malloc(sizeof(*A) * m * n);
    C = malloc(sizeof(*C) * m * p);
    B = malloc(sizeof(*B) * n * p);

    rand_matrix(A, m * n);
    rand_matrix(B, n * p);
}

// Structure for passing data to threads
typedef struct {
    size_t start;
    size_t end;
    size_t m;
    size_t n;
    size_t p;
} Args;

// Function to perform matrix multiplication for a given range of rows
void* worker(void* args_) {
    Args* args = args_;

    const size_t end = args->end;
    const size_t m   = args->m;
    const size_t n   = args->n;
    const size_t p   = args->p;
    printf("\t\t\t%4ld %4ld %4ld %4ld %4ld\n", m, n, p, args->start, args->end);

    register size_t C_sub = args->start, A_sub = args->start * n;

    for (size_t j = 0; j < p; j++, C_sub++) {
        // printf("Accessing: %4ld, %ld, %ld, %ld\n", i * m + j, i, m, j);
        C[C_sub] = 0;
        for (size_t k = 0; k < n; k++) 
            C[C_sub] += A[A_sub++] * B[k * p + j];
    }
    

    return NULL;
}

void compute(const size_t m, const size_t n, const size_t p, const size_t threads) {
    pthread_t* tids = malloc(sizeof(*tids) * threads); 

    // Create and join threads for matrix multiplication
    Args* args = malloc(sizeof(*args) * threads);

    const size_t thread_rows = m / threads;
    for (size_t i = 0; i < threads; i++) {
        args[i].start = i * thread_rows;
        args[i].end   = (i + 1) * thread_rows;
        // args[i].start = i;
        // args[i].end   = (i + 1);
        printf("%4ld %4ld\n", args[i].start, args[i].end);
        args[i].m     = m;
        args[i].n     = n;
        args[i].p     = p;
        pthread_create(&tids[i], NULL, worker, &args[i]);
    }

    for (size_t i = 0; i < threads; i++)
        ASSERT(pthread_join(tids[i], NULL));

    free(args);
    free(tids);
}

int main(const int argc, const char* argv[]) {

    if (argc != 5)
        USAGE()
    
    const size_t m       = atol(argv[1]);
    const size_t n       = atol(argv[2]);
    const size_t p       = atol(argv[3]);
    const size_t threads = atol(argv[4]);

    
    double start, finish;

    GET_TIME(start);
    matrices_init(m, n, p);
    GET_TIME(finish);

    printf("Initialaziation took %8.6f seconds\n", finish - start);
    
    GET_TIME(start);
    compute(m, n, p, threads);
    GET_TIME(finish);
    
    printf("Matrix Multiplication took %8.6f seconds\n", finish - start);
    


    return 0;
}



