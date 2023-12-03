# Compiler
CC = gcc

# Paths
COMMON_INCS	 := ./include/
COMMON_SRCS  := ./src/my_rand.c ./src/pth_ll_rwl.c
COMMON_OBJS  := $(subst .c,.o,$(COMMON_SRCS))

RP_SRCS  :=  ./src/rp_locks.c $(COMMON_SRCS)
RP_OBJS  := $(subst .c,.o,$(RP_SRCS))

WP_SRCS  :=  ./src/wp_locks.c $(COMMON_SRCS)
WP_OBJS  := $(subst .c,.o,$(WP_SRCS))

CFLAGS := -Wall -Wextra -I$(COMMON_INCS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -f $(COMMON_OBJS) $(RP_OBJS) $(WP_OBJS)
	@rm -f rp wp

rp: $(RP_OBJS)
	$(CC) -lpthread $^ -o ./rp

wp: $(WP_OBJS)
	$(CC) -lpthread $^ -o ./wp

all: 
	make -s rp
	make -s wp
