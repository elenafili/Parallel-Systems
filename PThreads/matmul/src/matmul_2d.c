#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "timer.h"
#include "my_rand.h"

#define USAGE() {                                                                       \
    fprintf(stderr, "Usage: %s <m> <n> <p> <threads> <log file, optional>\n", argv[0]); \
    fprintf(stderr, "       m, n, p > 0 and threads >= 1\n");                           \
    fprintf(stderr, "       m should be divisible by threads >= 1\n");                  \
    exit(EXIT_FAILURE);                                                                 \
}

#define ASSERT(call)             \
    if (call < 0) {              \
        perror(#call " failed"); \
        exit(EXIT_FAILURE);      \
    }                            \

size_t m, n, p, threads;
double** A;
double** B;
double** C;

void write_matrix(double** mat, const size_t m, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < m; i++) 
        write(fd, mat[i], n * sizeof(double));

    close(fd);
}

void rand_matrix(double** mat, const size_t m, const size_t n) {
    unsigned int seed = time(NULL);
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++)
            mat[i][j] = my_drand(&seed);
    }
}

void matrices_init() {
    A = malloc(sizeof(*A) * m);
    C = malloc(sizeof(*C) * m);
    B = malloc(sizeof(*B) * n);

    for (size_t i = 0; i < m; i++) {
        A[i] = malloc(sizeof(**A) * n);
        C[i] = malloc(sizeof(**C) * p);
    }

    for (size_t i = 0; i < n; i++)
        B[i] = malloc(sizeof(**B) * p);

    rand_matrix(A, m, n);
    rand_matrix(B, n, p);
}

typedef struct {
    size_t start;
    size_t end;
} Args;

void* worker(void* args_) {
    const Args* args = args_;

    for (size_t i = args->start; i < args->end; i++) {

        for (size_t j = 0; j < p; j++) {

            C[i][j] = 0;
            for (size_t k = 0; k < n; k++) 
                C[i][j] += A[i][k] * B[k][j];
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
        args[i].end   = (i + 1) * thread_rows;
        ASSERT(pthread_create(&tids[i], NULL, worker, &args[i]));
    }

    for (size_t i = 0; i < threads; i++)
        ASSERT(pthread_join(tids[i], NULL));

    free(args);
    free(tids);
}

int main(const int argc, const char* argv[]) {

    if (argc < 5)
        USAGE()
    
    m = atol(argv[1]);
    n = atol(argv[2]);
    p = atol(argv[3]);
    threads = atol(argv[4]);

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
        write_matrix(A, m, n, "./files/A.bin");
        write_matrix(B, n, p, "./files/B.bin");
        write_matrix(C, m, p, "./files/C.bin");
    #endif

    if (argc == 6) {
        FILE* file = fopen(argv[5], "a");

        fprintf(file, "2d,%ld,0,%f,%ld,%ld,%ld\n", threads, finish - start, m, n, p);

        fclose(file);
    }
    
    return 0;
}



