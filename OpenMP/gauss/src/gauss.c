#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <omp.h>

#include "my_rand.h"
#include "timer.h"


/* Serial functions */
void Get_args(int argc, char* argv[]);
void Usage(char* prog_name);
void Gen_matrix(double** A, size_t n);
void Gen_vector(double* x, size_t n);
void write_matrix(double** mat, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	
	for (size_t i = 0; i < n; i++)
		write(fd, mat[i], n * sizeof(double));

    close(fd);
}
void write_vector(double* vec, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	
	write(fd, vec, n * sizeof(double));

    close(fd);
}


/* Parallel function */
void gauss(uint8_t bool);

double** A;
double* x;
double* b;
size_t n, thread_count;

int main(int argc, char* argv[]) {

	Get_args(argc, argv);

	A = malloc(n * sizeof(double*));
	for (size_t i = 0; i < n; i++)
		A[i] = malloc(n * sizeof(double));

	x = malloc(n * sizeof(double));
	b = malloc(n * sizeof(double));
	
	Gen_matrix(A, n);
	Gen_vector(b, n);

    #ifdef VERIFY
        write_matrix(A, n, "./files/A.bin");
        write_vector(b, n, "./files/b.bin");
    #endif


    double start1, finish1;
    double start2, finish2;

    GET_TIME(start1);
	gauss(1);
    GET_TIME(finish1);

    printf("Trigonalization: %.4f sec\n", finish1 - start1);

    GET_TIME(start2);
	gauss(0);
    GET_TIME(finish2);

    printf("Solution: %.4f sec\n", finish2 - start2);

    #ifdef VERIFY
        write_matrix(A, n, "./files/A_trig.bin");
        write_vector(b, n, "./files/b_solv.bin");
        write_vector(x, n, "./files/x.bin");
    #endif

    if (argc == 6) {
        // FILE* file = fopen(argv[5], "a");

        // fprintf(file, "%d,%ld,%ld,%f,%f\n", thread_count, finish - start);

        // fclose(file);
    }

   free(A);
   free(x);
   free(b);

   return 0;
}  /* main */



void Get_args(int argc, char* argv[])  {

   if (argc < 3) Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);
   n = strtol(argv[2], NULL, 10);
   if (thread_count <= 0 || n <= 0) Usage(argv[0]);

}  /* Get_args */

void Usage (char* prog_name) {
   fprintf(stderr, "usage: %s <thread_count> <n> <schedule> <chunk_size> <log file, optional>\n", prog_name);
   exit(0);
}  /* Usage */



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


#ifdef TRI1
    #define PARALLEL1 _Pragma("omp parallel for num_threads(thread_count)")
#else
    #define PARALLEL1
#endif
#ifdef TRI2
    #define PARALLEL2 _Pragma("omp parallel for num_threads(thread_count)")
#else
    #define PARALLEL2
#endif


void gauss(uint8_t bool) {	
	if (bool) {
		for (size_t i = 0; i < n - 1; i++) {

			PARALLEL1
			for (size_t j = i + 1; j < n; j++) {
				double ratio = A[j][i] / A[i][i];

				PARALLEL2
				for (size_t k = i; k < n; k++)	
					A[j][k] -= ratio * A[i][k];

				b[j] -= ratio * b[i];
			}
		}
	}
	else {
        #ifdef REV
            double sum;

            #pragma omp parallel num_threads(thread_count)
            for (size_t temp = 0, row = n - 1; temp < n; row--, temp++) {
                
                #pragma omp single 
                {
                    sum = 0;
                }

                #pragma omp for reduction(+: sum)
                for (size_t col = row + 1; col < n; col++)
                    sum += A[row][col] * x[col];
                
                #pragma omp single 
                {
                    x[row] = (b[row] - sum) / A[row][row];
                }
            }
        #else
            for (size_t temp = 0, row = n - 1; temp < n; row--, temp++) {
                x[row] = b[row];

                for (size_t col = row + 1; col < n; col++)
                    x[row] -= A[row][col] * x[col];
                
                x[row] /= A[row][row];
            }
        #endif
	}
}

// PS1='${PWD##*/} $ '