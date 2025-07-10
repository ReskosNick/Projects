#!/bin/bash

# Specify the range of inputs and threads to test
n_values=(500 1000 2000 5000 8000 10000)
thread_values=(1 2 4)
approaches=(0 1)

# Specify the number of times to run the program for each input and thread count
num_runs=5


for n in "${n_values[@]}"; do
    for approach in "${approaches[@]}"; do
        for thread_count in "${thread_values[@]}"; do
            # Skip combinations where approach is Serial and thread_count is not 1,
            # approach is parallel and thread_count is equal to 1
            if ( [ "$approach" -eq 0 ] && [ "$thread_count" -ne 1 ] ) || ( [ "$approach" -eq 1 ] && [ "$thread_count" -eq 1 ] ); then
                continue
            fi 
            # Run the program multiple times for the current inputs
            for ((i = 1; i <= num_runs; i++)); do
                ./exercise1_6 $thread_count $n $approach
            done
        done
    done
done