# Compiler
CC = gcc

# Paths
INC	 := ./include/

MVM_SRCS  :=  ./src/omp_mat_vect_rand_split.c ./src/my_rand.c 
MVM_OBJS  := $(subst .c,.o,$(MVM_SRCS))

DEFINES = FIX
CFLAGS := -fopenmp -Wall -Wextra -I$(INC) -D$(DEFINES) -std=c11

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -f $(MVM_OBJS)
	@rm -f mvm

mvm: $(MVM_OBJS)
	$(CC) -fopenmp $^ -o ./mvm
