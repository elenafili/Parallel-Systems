all: false_sharing 2d

false_sharing: matmul_false_sharing.c my_rand.c
	gcc -g3 -Wall -std=c11 -lpthread matmul_false_sharing.c my_rand.c -o false_sharing

2d: matmul_false_sharing.c my_rand.c
	gcc -g3 -Wall -std=c11 -lpthread matmul_2d.c my_rand.c -o 2d

clean:
	rm -f false_sharing 2d