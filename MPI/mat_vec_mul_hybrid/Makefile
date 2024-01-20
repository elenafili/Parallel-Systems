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
    # NAMES := $(shell seq -w 1 15 | xargs printf "linux%02g," | sed 's/,$$//'),linux17
    NAMES := linux01,linux02,linux03,linux04,linux05,linux06,linux07,linux08,linux09,linux11,linux12,linux14,linux15,linux16,linux18,linux19
else
    NAMES := $(shell seq -w 1 $(M) | xargs printf "linux%02g," | sed 's/,$$//')
endif

mvm: ./src/*.c
	@mpicc -fopenmp -g -Wall ./src/*.c -lm -I./include/ -DN=$(SIZE) -o mvm

single-node:
	@mpiexec -n $(N) mvm

multi-node:
	@mpiexec -hosts $(NAMES) -n $(N) mvm

clean:
	rm -f mvm ./src/*.o