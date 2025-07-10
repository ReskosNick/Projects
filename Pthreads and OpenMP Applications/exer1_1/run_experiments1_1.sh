#!/bin/bash

# Specify the range of inputs and threads to test
throw_values=(100000000 1000000000)
thread_values=(1 2 4 8)

# Specify the number of times to run the program for each input and thread count
num_runs=5


for throw_num in "${throw_values[@]}"; do
    for thread_count in "${thread_values[@]}"; do
        # Run the program multiple times for the current inputs
        for ((i = 1; i <= num_runs; i++)); do
            ./exercise1_1 $throw_num $thread_count
        done
    done
done
