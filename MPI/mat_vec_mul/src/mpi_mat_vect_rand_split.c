#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "my_rand.h"

#ifndef N
	#define N 15000
#endif

// Functions for initial node to generate matrix and vector
void generate_matrix(double* Matrix, const size_t n, int columns, int local_n, int block_size);
void generate_vector(double* vector, const size_t n);

// Functions for printing and debugging
void print_matrix(double* matrix, const size_t n, int columns, int local_n, int block_size);
void print_vector(double* vector, const size_t n);


double my_abs(double a) {
    return a >= 0 ? a : -a; 
}

double* reshape(double* A, int columns, int local_n, int block_size) {
    double* B = malloc(sizeof(double) * N * N);

    for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			int skip = i / local_n * columns + j / local_n;
			int offset = i % local_n * local_n + j % local_n;
			B[i * N + j] = A[skip * block_size + offset];
		}
    }

    return B;
}

int main(int argc, char *argv[]) {
	double start = 0, finish = 0, loc_elapsed = 0, elapsed = 0;
	double start_after = 0, loc_elapsed_after = 0, elapsed_after = 0;

	MPI_Init(&argc, &argv);
	
	int rank, comm_sz, columns, local_n, block_size;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	double* Matrix = NULL;
	double* Vector = NULL;
	double* Product = NULL;

	if (rank == 0) {
		Matrix = calloc(N * N, sizeof(double));
		Vector = calloc(N, sizeof(double));
		Product = calloc(N, sizeof(double));

		if (Matrix == NULL || Vector == NULL || Product == NULL) {
			printf("calloc failed on rank: %d!\n", rank);
			MPI_Finalize();
			exit(-1);
		}
	}

	
	columns = (int)sqrt(comm_sz);

	if (columns * columns != comm_sz) {
		printf("comm_sz = %d should be a perfect square!\n", comm_sz);
		MPI_Finalize();
		exit(-1);
	}


	local_n = N / columns;
	block_size = local_n * local_n;


	int col_id = rank / columns;
	int col_rank;
	
	MPI_Comm col_comm;
	MPI_Comm_split(MPI_COMM_WORLD, col_id, rank, &col_comm);
	MPI_Comm_rank(col_comm, &col_rank);


	int row_id = rank % columns;
	int row_rank;

	MPI_Comm row_comm;
	MPI_Comm_split(MPI_COMM_WORLD, row_id, rank, &row_comm);
	MPI_Comm_rank(row_comm, &row_rank);


	if (rank == 0) {
		generate_matrix(Matrix, N, columns, local_n, block_size);
		generate_vector(Vector, N);
	}

	if (N % columns != 0) {
		printf("Square root of comm_sz should divide matrix size\n");
		MPI_Comm_free(&col_comm);
		MPI_Comm_free(&row_comm);
		MPI_Finalize();
		exit(-1);
	}

	double* Local_Matrix = calloc(block_size, sizeof(double));
	double* Local_Vector = calloc(local_n, sizeof(double));
	double* Head_Product = calloc(local_n, sizeof(double));
	double* Local_Product = calloc(local_n, sizeof(double));

	if (Local_Matrix == NULL || Local_Vector == NULL || 
		Head_Product == NULL || Local_Product == NULL) {
		printf("calloc failed on rank: %d!\n", rank);
		MPI_Finalize();
		exit(-1);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();

	MPI_Scatter(Matrix, block_size, MPI_DOUBLE, Local_Matrix, block_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	
	// Scatter vector to the processes
	if (rank < columns) 
		MPI_Scatter(Vector, local_n, MPI_DOUBLE, Local_Vector, local_n, MPI_DOUBLE, 0, col_comm);

	MPI_Bcast(Local_Vector, local_n, MPI_DOUBLE, 0, row_comm);

	// Scatter product to the processes
	if (rank % columns == 0) 
		MPI_Scatter(Product, local_n, MPI_DOUBLE, Head_Product, local_n, MPI_DOUBLE, 0, row_comm);

	start_after = MPI_Wtime();

	for (int i = 0; i < local_n; i++) {
		for (int j = 0; j < local_n; j++)
			Local_Product[i] += Local_Matrix[i * local_n + j] * Local_Vector[j];
	}

	finish = MPI_Wtime();
	loc_elapsed_after = finish - start_after;

	MPI_Reduce(Local_Product, Head_Product, local_n, MPI_DOUBLE, MPI_SUM, 0, col_comm);

	if (rank % columns == 0) 
		MPI_Gather(Head_Product, local_n, MPI_DOUBLE, Product, local_n, MPI_DOUBLE, 0, row_comm);

	finish = MPI_Wtime();
	loc_elapsed = finish - start;
	
	free(Local_Matrix);
	free(Local_Vector);
	free(Head_Product);
	free(Local_Product);

	MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&loc_elapsed_after, &elapsed_after, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (rank == 0)
		printf("Elapsed time = %e,%e\n", elapsed, elapsed_after);

	if (rank == 0) {
		// printf("\nThe Matrix \n");
		// print_matrix(Matrix, N, columns, local_n, block_size);
		
		// printf("The Vector \n");
		// print_vector(Vector, N);

		// printf("The Product \n");
		// print_vector(Product, N);

		double* A = reshape(Matrix, columns, local_n, block_size);
		free(Matrix);
		Matrix = A;

		double diff = 0;
		for (int i = 0; i < N; i++) {
			double element = 0;
			for (int j = 0; j < N; j++)
				element += Matrix[i * N + j] * Vector[j];
				
			diff += my_abs(element - Product[i]);
		}

		printf("||Ax - y||_1 = %e\n", diff);

		free(Matrix);
		free(Vector);
		free(Product);
	}


	MPI_Comm_free(&col_comm);
	MPI_Comm_free(&row_comm);
	MPI_Finalize();

	return 0;
}

  
void generate_matrix(double* matrix, const size_t n, int columns, int local_n, int block_size){

	unsigned int seed = time(NULL) ^ n;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			int skip = i / local_n * columns + j / local_n;
			int offset = i % local_n * local_n + j % local_n;
			matrix[skip * block_size + offset] = my_drand(&seed);
		}
	}
}

void generate_vector(double* vector, const size_t n) {

	unsigned int seed = time(NULL) ^ n;
	
	for (int i = 0; i < n; i++)
		vector[i] = my_drand(&seed);
}

void print_matrix(double* matrix, const size_t n, int columns, int local_n, int block_size){

	for (int i = 0; i < n; i++) {

		for (int j = 0; j < n; j++) {
			int skip = i / local_n * columns + j / local_n;
			int offset = i % local_n * local_n + j % local_n;
			printf("%11.8lf ", matrix[skip * block_size + offset]);
		}

		printf("\n");
	}

	printf("\n");
}

void print_vector(double* vector, const size_t n) {
	for (int i = 0; i < n; i++)
		printf("%11.8lf ", vector[i]);

	printf("\n");
}