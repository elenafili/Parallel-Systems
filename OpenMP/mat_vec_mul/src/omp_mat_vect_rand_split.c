#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <omp.h>

#include "my_rand.h"
#include "timer.h"

typedef enum {
   BASELINE,
   AUTO,
   STATIC,
   STATIC_CS,
   DYNAMIC,
   DYNAMIC_CS,
   GUIDED,
   GUIDED_CS,
} SCH;


void write_vector(double* vec, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	
	write(fd, vec, n * sizeof(double));

    close(fd);
}


void Gen_matrix(double* A, size_t n) {
	unsigned int seed = 8;

	for (size_t i = 0; i < n; i++) {

		for (size_t j = 0; j < i; j++)
			A[i * n + j] = 0;

		for (size_t j = i; j < n; j++)
			A[i * n + j] = my_drand(&seed);

	}
}

void Gen_vector(double* x, size_t n) {
   unsigned int seed = 9;

   for (size_t i = 0; i < n; i++)
      x[i] = my_drand(&seed);
} 


#define LOOP                           		\
	for (i = 0; i < n; i++) {    			\
		y[i] = 0.0;                      	\
		for (j = i; j < n; j++) { 			\
			temp = A[i * n + j] * x[j];   	\
			y[i] += temp;                 	\
		}                                	\
	}                                   	\

void Omp_mat_vect(double* A, double* x, double* y, size_t n, size_t thread_count, SCH sch_type, size_t chunk_size) {
   	double temp;
	size_t i, j;

	switch (sch_type) {
		case AUTO:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n)  schedule(auto)
			LOOP
			break;
		case STATIC:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n) schedule(static)
			LOOP
			break;
		case STATIC_CS:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n, chunk_size) schedule(static, chunk_size)
			LOOP
			break;
		case DYNAMIC:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n) schedule(dynamic)
			LOOP
			break;
		case DYNAMIC_CS:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n, chunk_size) schedule(dynamic, chunk_size)
			LOOP
			break;
		case GUIDED:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n) schedule(guided)
			LOOP
			break;
		case GUIDED_CS:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n, chunk_size) schedule(guided, chunk_size)
			LOOP
			break;
		case BASELINE:
		default:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n)
			LOOP
			break;
	}
}


void Get_args(int argc, char* argv[], size_t* thread_count_p, size_t* n_p, SCH* sch_type, size_t* chunk_size)  {

	if (argc < 5) 
		Usage(argv[0]);

	*thread_count_p = strtol(argv[1], NULL, 10);
	*n_p = strtol(argv[2], NULL, 10);

	size_t temp = strtol(argv[3], NULL, 10);

	*sch_type = temp > 7 ? 0 : temp;
	*chunk_size = strtol(argv[4], NULL, 10);
	
	if (*thread_count_p <= 0 || *n_p <= 0) 
		Usage(argv[0]);

}

void Usage(char* prog_name) {
	fprintf(stderr, "usage: %s <threads> <n> <sch type> <chunk size> <log file, optional>\n", prog_name);
	exit(0);
}

int main(int argc, char* argv[]) {
	double* A;
	double* x;
	double* y;
	SCH sch_type;
	size_t n, thread_count, chunk_size;

	Get_args(argc, argv, &thread_count, &n, &sch_type, &chunk_size);

	A = malloc(n * n * sizeof(double));
	x = malloc(n * sizeof(double));
	y = malloc(n * sizeof(double));
   
	Gen_matrix(A, n);
	Gen_vector(x, n);

    double start, finish;

    GET_TIME(start);
   	Omp_mat_vect(A, x, y, n, thread_count, sch_type, chunk_size);
    GET_TIME(finish);

    #ifdef VERIFY
        write_vector(A, n * n, "./files/A.bin");
        write_vector(x, n, "./files/x.bin");
        write_vector(y, n, "./files/y.bin");
    #endif

    if (argc == 6) {
        FILE* file = fopen(argv[5], "a");

        fprintf(file, "%d,%ld,%ld,%f\n", sch_type, chunk_size, thread_count, finish - start);

        fclose(file);
    }

   free(A);
   free(x);
   free(y);

   return 0;
}