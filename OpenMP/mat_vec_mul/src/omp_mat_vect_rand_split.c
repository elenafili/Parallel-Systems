/* File:     
 *     omp_mat_vect_rand_split.c 
 *
 * Purpose:  
 *     Computes a parallel matrix-vector product.  Matrix
 *     is distributed by block rows.  Vectors are distributed by 
 *     blocks.  This version uses a random number generator to
 *     generate A and x.  There is some optimization.
 *
 * Compile:  
 *    gcc -g -Wall -fopenmp -o omp_mat_vect_rand_split 
 *          omp_mat_vect_rand_split.c 
 * Run:
 *    ./omp_mat_vect_rand_split <thread_count> <m> <n>
 *
 * Input:
 *     None unless compiled with DEBUG flag.
 *     With DEBUG flag, A, x
 *
 * Output:
 *     y: the product vector
 *     Elapsed time for the computation
 *
 * Notes:  
 *     1.  Storage for A, x, y is dynamically allocated.
 *     2.  Number of threads (thread_count) should evenly divide both 
 *         m and n.  The program doesn't check for this.
 *     3.  We use a 1-dimensional array for A and compute subscripts
 *         using the formula A[i][j] = A[i*n + j]
 *     4.  Distribution of A, x, and y is logical:  all three are 
 *         globally shared.
 *     5.  DEBUG compile flag will prompt for input of A, x, and
 *         print y
 *
 * IPP:  Exercise 5.12
 */

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
	AUTO,
	STATIC,
	STATIC_CS,
	DYNAMIC,
	DYNAMIC_CS,
	GUIDED,
	GUIDED_CS,
	RUNTIME
} SCH;

/* Serial functions */
void Get_args(int argc, char* argv[], size_t* thread_count_p, size_t* n_p, SCH* sch_type, size_t* chunk_size);
void Usage(char* prog_name);
void Gen_matrix(double A[], size_t n);
void Read_matrix(char* prompt, double A[], size_t n);
void Gen_vector(double x[], size_t n);
void Read_vector(char* prompt, double x[], size_t n);
void Print_matrix(char* title, double A[], size_t n);
void Print_vector(char* title, double y[], double n);

/* Parallel function */
void Omp_mat_vect(double A[], double x[], double y[], 
				size_t n, size_t thread_count, SCH sch_type, size_t chunk_size);


void write_vector(double* vec, const size_t n, const char* path) {

	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
    }
	
	write(fd, vec, n * sizeof(double));

	close(fd);
}

/*------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
	double* A;
	double* x;
	double* y;
	SCH sch_type;
	size_t n, thread_count, chunk_size;

	Get_args(argc, argv, &thread_count, &n, &sch_type, &chunk_size);

	A = malloc(n*n*sizeof(double));
	x = malloc(n*sizeof(double));
	y = malloc(n*sizeof(double));
   
	#ifdef DEBUG
		Read_matrix("Enter the matrix", A, n);
		Print_matrix("We read", A, n);
		Read_vector("Enter the vector", x, n);
		Print_vector("We read", x, n);
	#else
	Gen_matrix(A, n);
	/* Print_matrix("We generated", A, n); */
	Gen_vector(x, n);
	/* Print_vector("We generated", x, n); */
	#endif

	double start, finish;

	GET_TIME(start);
	Omp_mat_vect(A, x, y, n, thread_count, sch_type, chunk_size);
	GET_TIME(finish);

	#ifdef DEBUG
	Print_vector("The product is", y, n);
	#else
	/* Print_vector("The product is", y, n); */
	#endif


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
	}  /* main */


/*------------------------------------------------------------------
 * Function:  Get_args
 * Purpose:   Get command line args
 * In args:   argc, argv
 * Out args:  thread_count_p, m_p, n_p
 */
void Get_args(int argc, char* argv[], size_t* thread_count_p, size_t* n_p, SCH* sch_type, size_t* chunk_size)  {
	if (argc < 5) Usage(argv[0]);

	*thread_count_p = strtol(argv[1], NULL, 10);
	*n_p = strtol(argv[2], NULL, 10);

	size_t temp = strtol(argv[3], NULL, 10);

	*sch_type = temp > 7 ? 0 : temp;
	*chunk_size = strtol(argv[4], NULL, 10);

	if (*thread_count_p <= 0 || *n_p <= 0) Usage(argv[0]);
}  /* Get_args */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   print a message showing what the command line should
 *            be, and terminate
 * In arg :   prog_name
 */
void Usage (char* prog_name) {
	fprintf(stderr, "usage: %s <thread_count> <n> <schedule> <chunk_size> <log file, optional>\n", prog_name);
	exit(0);
}  /* Usage */

/*------------------------------------------------------------------
 * Function:    Read_matrix
 * Purpose:     Read in the matrix
 * In args:     prompt, m, n
 * Out arg:     A
 */
void Read_matrix(char* prompt, double A[], size_t n) {
	printf("%s\n", prompt);

	for (size_t i = 0; i < n; i++) 
		for (size_t j = 0; j < n; j++)
			scanf("%lf", &A[i*n+j]);
}  /* Read_matrix */

/*------------------------------------------------------------------
 * Function: Gen_matrix
 * Purpose:  Use the random number generator random to generate
 *    the entries in A
 * In args:  m, n
 * Out arg:  A
 */
void Gen_matrix(double A[], size_t n) {
	unsigned int seed = 8;
	for (size_t i = 0; i < n; i++) {
		
		for (size_t j = 0; j < i; j++)
			A[i * n + j] = 0;

		for (size_t j = i; j < n; j++)
			A[i * n + j] = my_drand(&seed);

	}
}  /* Gen_matrix */

/*------------------------------------------------------------------
 * Function: Gen_vector
 * Purpose:  Use the random number generator random to generate
 *    the entries in x
 * In arg:   n
 * Out arg:  A
 */
void Gen_vector(double x[], size_t n) {
	unsigned int seed = 9;
	for (size_t i = 0; i < n; i++)
		x[i] = my_drand(&seed);
}  /* Gen_vector */

/*------------------------------------------------------------------
 * Function:        Read_vector
 * Purpose:         Read in the vector x
 * In arg:          prompt, n
 * Out arg:         x
 */
void Read_vector(char* prompt, double x[], size_t n) {
	printf("%s\n", prompt);
	for (size_t i = 0; i < n; i++) 
		scanf("%lf", &x[i]);
}  /* Read_vector */


/*------------------------------------------------------------------
 * Function:  Omp_mat_vect
 * Purpose:   Multiply an mxn matrix by an nx1 column vector
 * In args:   A, x, m, n, thread_count
 * Out arg:   y
 */
#define LOOP                           		\
	for (i = 0; i < n; i++) {    			\
		y[i] = 0.0;                      	\
		for (j = 0; j < n; j++) { 			\
			temp = A[i * n + j] * x[j];   	\
			y[i] += temp;                 	\
		}                                	\
	}                                   	\

void Omp_mat_vect(double A[], double x[], double y[], 
				size_t n, size_t thread_count, SCH sch_type, size_t chunk_size) {
	double temp;
	size_t i, j;

	switch (sch_type) {
		case AUTO:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n) schedule(auto)
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
		case RUNTIME:
		default:
			#pragma omp parallel for num_threads(thread_count) default(none) \
					private(i, j, temp) shared(A, x, y, n) schedule(runtime)
			LOOP
			break;
	}
}  /* Omp_mat_vect */


/*------------------------------------------------------------------
 * Function:    Print_matrix
 * Purpose:     Print the matrix
 * In args:     title, A, m, n
 */
void Print_matrix( char* title, double A[], size_t n) {
	printf("%s\n", title);
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++)
			printf("%4.1f ", A[i*n + j]);
		printf("\n");
	}
}  /* Print_matrix */


/*------------------------------------------------------------------
 * Function:    Print_vector
 * Purpose:     Print a vector
 * In args:     title, y, m
 */
void Print_vector(char* title, double y[], double m) {
	printf("%s\n", title);
	for (size_t i = 0; i < m; i++)
		printf("%4.1f ", y[i]);
	printf("\n");
}  /* Print_vector */
