all: monte_carlo

monte_carlo: monte_carlo.c
	mpicc -Wall -Wextra -pthread monte_carlo.c -o monte_carlo
	
clean:
	rm -f monte_carlo