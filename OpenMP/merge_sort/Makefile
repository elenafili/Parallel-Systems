# Compiler
CC = gcc

# Paths
INC	 := ./include/

MS_SRCS  :=  ./src/merge_sort.c ./src/my_rand.c 
MS_OBJS  := $(subst .c,.o,$(MS_SRCS))

DEFINES = VERIFY
CFLAGS := -fopenmp -Wall -Wextra -I$(INC) -D$(DEFINES) -std=c11 -O3

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -f $(MS_OBJS)
	@rm -f merge_sort

merge_sort: $(MS_OBJS)
	$(CC) -fopenmp $^ -o ./merge_sort