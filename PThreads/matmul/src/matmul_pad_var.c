#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "timer.h"
#include "my_rand.h"

#define USAGE() {                                                           \
    fprintf(stderr, "Usage: %s <m> <n> <p> <pad> <threads>\n", argv[0]);    \
    fprintf(stderr, "       m, n, p pad, threads > 0\n");                   \
    fprintf(stderr, "       m should be divisible by threads >= 1\n");      \
    exit(EXIT_FAILURE);                                                     \
}

#define ASSERT(call)             \
    if (call < 0) {              \
        perror(#call " failed"); \
        exit(EXIT_FAILURE);      \
    }                            \

size_t m, n, p, pad, threads;
double* A;
double* B;
double* C;

void write_matrix(double* mat, const size_t size, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    if (mat != C)
        write(fd, mat, size * sizeof(double));
    else {
        double* line = malloc(p * sizeof(double));
        
        for (size_t i = 0; i < m * pad; i += pad) {

            for (size_t j = 0; j < p; j++) 
                line[j] = mat[i * p + j];

            write(fd, line, p * sizeof(double));
        }


        free(line);
    }

    close(fd);
}

void rand_matrix(double* mat, const size_t m) {
    unsigned int seed = time(NULL);
    for (size_t i = 0; i < m; i++) 
        mat[i] = my_drand(&seed);
} 

void matrices_init() {
    A = malloc(sizeof(*A) * m * n);
    C = malloc(sizeof(*C) * m * p * pad);
    B = malloc(sizeof(*B) * n * p);

    rand_matrix(A, m * n);
    rand_matrix(B, n * p);
}

typedef struct {
    size_t start;
    size_t end;
} Args;

void* worker(void* args_) {
    const Args* args = args_;

    register size_t C_sub = args->start * p * pad, A_sub = args->start * n;

    for (size_t i = args->start; i < args->end; i++, A_sub += n, C_sub += p * (pad - 1)) { 

        for (size_t j = 0; j < p; j++, C_sub++) {
            C[C_sub] = 0;

            for (size_t k = 0; k < n; k++) 
                C[C_sub] += A[A_sub + k] * B[k * p + j];
        }
    }
    
    return NULL;
}

void compute() {
    pthread_t* tids = malloc(sizeof(*tids) * threads); 

    Args* args = malloc(sizeof(*args) * threads);

    const size_t thread_rows = m / threads;

    for (size_t i = 0; i < threads; i++) {
        
        args[i].start = i * thread_rows;
        args[i].end = (i + 1) * thread_rows;

        ASSERT(pthread_create(&tids[i], NULL, worker, &args[i]));
    }

    for (size_t i = 0; i < threads; i++)
        ASSERT(pthread_join(tids[i], NULL));

    free(args);
    free(tids);
}

int main(const int argc, const char* argv[]) {

    if (argc != 6)
        USAGE()
    
    m = atol(argv[1]);
    n = atol(argv[2]);
    p = atol(argv[3]);
    pad = atol(argv[4]);
    threads = atol(argv[5]);

    
    double start, finish;

    GET_TIME(start);
    matrices_init();
    GET_TIME(finish);

    printf("Initialaziation took %8.6f seconds\n", finish - start);
    
    GET_TIME(start);
    compute();
    GET_TIME(finish);
    
    printf("Matrix Multiplication took %8.6f seconds\n", finish - start);

    #ifdef VERIFY
        write_matrix(A, m * n, "./files/A.bin");
        write_matrix(B, n * p, "./files/B.bin");
        write_matrix(C, m * p * pad, "./files/C.bin");
    #endif

    return 0;
}

