# Compiler
CC = gcc

# Paths
INC	 := ./include/

GAUSS_SRCS  :=  ./src/gauss.c ./src/my_rand.c 
GAUSS_OBJS  := $(subst .c,.o,$(GAUSS_SRCS))

DEFINES = -DVERIFY -DTRI2 -DREV # -DTRI1 -DTRI2 -DREV
CFLAGS := -fopenmp -Wall -Wextra -I$(INC) $(DEFINES) -std=c11 -O3 

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -f $(GAUSS_OBJS)
	@rm -f gauss

gauss: $(GAUSS_OBJS)
	$(CC) -fopenmp $^ -o ./gauss
