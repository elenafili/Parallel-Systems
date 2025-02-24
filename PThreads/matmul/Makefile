# Compiler
CC = gcc

# Paths
COMMON_INCS	 := ./include/
COMMON_SRCS  := ./src/my_rand.c
COMMON_OBJS  := $(subst .c,.o,$(COMMON_SRCS))

FALSE_SRCS  :=  ./src/matmul_false_sharing.c $(COMMON_SRCS)
FALSE_OBJS  := $(subst .c,.o,$(FALSE_SRCS))

2D_SRCS  :=  ./src/matmul_2d.c $(COMMON_SRCS)
2D_OBJS  := $(subst .c,.o,$(2D_SRCS))

PAD_VAR_SRCS  :=  ./src/matmul_pad_var.c $(COMMON_SRCS)
PAD_VAR_SRCS  := $(subst .c,.o,$(PAD_VAR_SRCS))

PRIV_VAR_SRCS  :=  ./src/matmul_private_var.c $(COMMON_SRCS)
PRIV_VAR_OBJS  := $(subst .c,.o,$(PRIV_VAR_SRCS))

DEFINES = VERIFY
CFLAGS := -Wall -Wextra -I$(COMMON_INCS) -D$(DEFINES)
LDFLAGS += -pthread

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	@rm -f $(COMMON_OBJS) $(FALSE_OBJS) $(2D_OBJS) $(PAD_VAR_SRCS) $(PRIV_VAR_OBJS)
	@rm -f fs 2d pad_var priv_var

fs: $(FALSE_OBJS)
	$(CC) -pthread $^ -o ./fs

2d: $(2D_OBJS)
	$(CC) -pthread $^ -o ./2d

pad_var: $(PAD_VAR_SRCS)
	$(CC) -pthread $^ -o ./pad_var

priv_var: $(PRIV_VAR_OBJS)
	$(CC) -pthread $^ -o ./priv_var

all: 
	make -s fs
	make -s 2d
	make -s pad_var
	make -s priv_var
