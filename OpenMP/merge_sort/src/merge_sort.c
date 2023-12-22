#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include "my_rand.h"
#include "timer.h"

void Usage(char* prog_name) {
	fprintf(stderr, "usage: %s <n> <threads> <log file, optional>\n", prog_name);
	exit(0);
}

void Get_args(int argc, char* argv[], size_t* n, size_t* threads)  {
    if (argc < 3) 
        Usage(argv[0]);

    *n = strtol(argv[1], NULL, 10);
    *threads = strtol(argv[2], NULL, 10);
	
    if (*threads <= 0 || *n <= 0) 
        Usage(argv[0]);
}

void gen_array(int* array, size_t n) {
	unsigned int seed = 15;

	for (size_t i = 0; i < n; i++) 
	    array[i] = my_rand(&seed);
}

void write_array(int* array, const size_t n, const char* path) {

    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
	
	assert(write(fd, array, n * sizeof(int)));

    close(fd);
}

void merge(int* array, size_t left, size_t middle, size_t right) {
    size_t left_num = middle - left + 1;
    size_t right_num = right - middle;

    int* left_array = malloc(left_num * sizeof(int));
    int* right_array = malloc(right_num * sizeof(int));

    for (size_t i = 0; i < left_num; i++) 
        left_array[i] = array[left + i];
    
    for (size_t i = 0; i < right_num; i++) 
        right_array[i] = array[middle + 1 + i];

    size_t i = 0, j = 0, k = left;

    while (i < left_num && j < right_num) 
        array[k++] = (left_array[i] < right_array[j]) ? left_array[i++] : right_array[j++];
    
    while (i < left_num)
        array[k++] = left_array[i++];

     while (j < right_num)
        array[k++] = right_array[j++];

    free(left_array);
    free(right_array);
}

void mergeSort(int* array, size_t left, size_t right, size_t threads) {
    if (left >= right) 
        return;

    size_t middle = (left + right) / 2;

    #pragma omp task if (threads > 1)
    mergeSort(array, left, middle, threads);

    #pragma omp task if (threads > 1)
    mergeSort(array, middle + 1, right, threads);

    // Wait for both tasks to complete before merging
    #pragma omp taskwait 
    merge(array, left, middle, right);
}

int main(int argc, char* argv[]) {
    int* array;
    size_t n, threads;

	Get_args(argc, argv, &n, &threads);

	array = malloc(n * sizeof(int));
	gen_array(array, n);

    #ifdef VERIFY  
        write_array(array, n, "./files/initialArray.bin");
    #endif

    double start, finish;

    GET_TIME(start);

   	#pragma omp parallel num_threads(threads)
    #pragma omp single
    mergeSort(array, 0, n-1, threads);

    GET_TIME(finish);

    printf("Time Elapsed %f\n", finish - start);

    #ifdef VERIFY
        write_array(array, n, "./files/sortedArray.bin");
    #endif

    if (argc == 4) {
        FILE* file = fopen(argv[3], "a");

        fprintf(file, "%f\n", finish - start);

        fclose(file);
    }

   free(array);
   return 0;
}
