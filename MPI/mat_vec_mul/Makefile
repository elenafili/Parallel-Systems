ifndef N
N := 4
endif

ifndef M
M := 4
endif

ifndef SIZE
SIZE := 1000
endif

ifeq ($(M), 16)
    NAMES := $(shell seq -w 1 15 | xargs printf "linux%02g," | sed 's/,$$//'),linux17
else
    NAMES := $(shell seq -w 1 $(M) | xargs printf "linux%02g," | sed 's/,$$//')
endif

mvm: ./src/*.c
	@mpicc -o mvm -g -Wall ./src/*.c -lm -I./include/ -DN=$(SIZE)

single-node:
	@mpiexec -n $(N) mvm

multi-node:
	@mpiexec -hosts $(NAMES) -n $(N) mvm

clean:
	rm -f mvm ./src/*.o