#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <omp.h>
#include <assert.h>

#include "my_rand.h"
#include "timer.h"

double* A;
double* x;
double* b;
size_t n, thread_count;

void write_matrix(double** mat, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	
	for (size_t i = 0; i < n; i++)
		assert(write(fd, mat[i], n * sizeof(double)) > 0);

    close(fd);
}

void write_vector(double* vec, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	
    assert(write(fd, vec, n * sizeof(double)) > 0);

    close(fd);
}


void Gen_matrix(double** A, size_t n) {
	unsigned int seed = 8;

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++)
			A[i][j] = my_drand(&seed);
	}
}

void Gen_vector(double* x, size_t n) {
	unsigned int seed = 9;
	
	for (size_t i = 0; i < n; i++)
		x[i] = my_drand(&seed);	
}


void gauss() {
    size_t i, j, k;
    double ratio;
    register size_t off1, off2;

    #ifdef TRI1
    #pragma omp parallel num_threads(thread_count) \
    default(none) private(i, j, k, ratio, off1, off2) shared(A, b, n)
    #elif TRI2
    #pragma omp parallel num_threads(thread_count) \
    default(none) private(i, j, k) shared(A, b, n, ratio, off1, off2)
    #endif
    for (i = 0; i < n - 1; i++) {

        
        off1 = i * n;

        #ifdef TRI1
        #pragma omp for
        #endif
        for (j = i + 1; j < n; j++) {

            #if defined(TRI2)
                #pragma omp single
                {
            #endif
                off2 = j * n;
                ratio = A[off2 + i] / A[off1 + i];
                b[j] -= ratio * b[i];
            #if defined(TRI2)
                }
            #endif
            

            #ifdef TRI2
            #pragma omp for nowait
            #endif
            for (k = i; k < n; k++)
                A[off2 + k] -= ratio * A[off1 + k];
            
        }
    }
}

void back() {
    #ifdef BACK
        double sum;

        #pragma omp parallel num_threads(thread_count)
        for (size_t temp = 0, row = n - 1; temp < n; row--, temp++) {
            
            #pragma omp single 
            sum = 0;
            
            #pragma omp for reduction(+: sum)
            for (size_t col = row + 1; col < n; col++)
                sum += A[row * n + col] * x[col];
            
            #pragma omp single 
            x[row] = (b[row] - sum) / A[row * n + row];
            
        }			
    #else
        for (size_t temp = 0, row = n - 1; temp < n; row--, temp++) {
            x[row] = b[row];

            for (size_t col = row + 1; col < n; col++)
                x[row] -= A[row * n + col] * x[col];
            
            x[row] /= A[row * n + row];
        }
    #endif
}

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <thread_count> <n> <schedule> <chunk_size> <log file, optional>\n", prog_name);
   exit(0);
}

void Get_args(int argc, char* argv[])  {

	if (argc < 3) 
		Usage(argv[0]);

	thread_count = strtol(argv[1], NULL, 10);
	n = strtol(argv[2], NULL, 10);

	if (thread_count <= 0 || n <= 0) 
		Usage(argv[0]);
}

int main(int argc, char* argv[]) {

	Get_args(argc, argv);

	A = malloc(n * n * sizeof(double));
	x = malloc(n * sizeof(double));
	b = malloc(n * sizeof(double));
	
	Gen_vector(A, n * n);
	Gen_vector(b, n);

    #ifdef VERIFY
        write_vector(A, n * n, "./files/A.bin");
        write_vector(b, n, "./files/b.bin");
    #endif

    double start1, finish1;
    double start2, finish2;

    GET_TIME(start1);
	gauss();
    GET_TIME(finish1);

    GET_TIME(start2);
	back();
    GET_TIME(finish2);

	printf("Trig: %f\nRev:  %f\n", finish1 - start1, finish2 - start2);

    #ifdef VERIFY
        write_vector(A, n * n, "./files/A_trig.bin");
        write_vector(b, n, "./files/b_solv.bin");
        write_vector(x, n, "./files/x.bin");
    #endif

    size_t id = 0;

    #ifdef TRI1
        id = 2;
    #endif
    #ifdef TRI2
        id = 4;
    #endif
    #ifdef BACK
        id++;
    #endif

    if (argc == 4) {
        FILE* file = fopen(argv[3], "a");

        fprintf(file, "%ld,%ld,%f,%f,%ld\n", n, thread_count, finish1 - start1, finish2 - start2, id);

        fclose(file);
    }

   free(A);
   free(x);
   free(b);

   return 0;
}