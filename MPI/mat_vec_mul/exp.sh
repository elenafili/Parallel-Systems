#!/bin/bash

output_file="output.txt"

SIZE="$1"

M_VALUES=(1 4 16)

for M in "${M_VALUES[@]}"; do
    if [ "$M" -eq 1 ]; then
		echo "Running with $SIZE,1,1" >> "$output_file"
		make single-node N=1 >> "$output_file" 2>&1
		echo "Running with $SIZE,1,4" >> "$output_file"
		make single-node N=4 >> "$output_file" 2>&1
    elif [ "$M" -eq 4 ]; then
		echo "Running with $SIZE,4,16" >> "$output_file"
		make multi-node M=4 N=16 >> "$output_file" 2>&1
    else
		echo "Running with $SIZE,16,64" >> "$output_file"
		make multi-node M=16 N=64 >> "$output_file" 2>&1
    fi
done
