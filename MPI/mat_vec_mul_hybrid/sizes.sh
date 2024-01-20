#!/bin/bash

output_file="output.txt"
# truncate --size 0 "$output_file"

SIZES=(1000 5000 10000 15000)

for SIZE in "${SIZES[@]}"; do
    make clean
    make mvm SIZE="$SIZE"
    
    for ((i=0; i<4; i++)); do
        ./exp.sh "$SIZE"
		# echo "Running with $SIZE,1,1" >> "$output_file"
		# make single-node N=1 >> "$output_file" 2>&1
    done
done
