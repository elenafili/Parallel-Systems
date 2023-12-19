
# Paths
INC := ./include/

GAUSS_SRCS := ./src/gauss.c ./src/my_rand.c
GAUSS_OBJS := $(subst .c,.o,$(GAUSS_SRCS))

CFLAGS := -fopenmp -Wall -Wextra -I$(INC) -std=c11 -O3

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -f $(GAUSS_OBJS)
	@rm -f gauss

.PHONY: gauss

gauss: CFLAGS += $(FLAG)
gauss: $(GAUSS_OBJS)
	$(CC) -fopenmp $^ -o ./gauss