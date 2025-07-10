#!/bin/bash

# Specify the range of inputs and threads to test
n_values=(8 80 800 8000)
thread_values=(1 2 4)

# Specify the number of times to run the program for each input and thread count
num_runs=5


for n in "${n_values[@]}"; do
    for thread_count in "${thread_values[@]}"; do
        # Run the program multiple times for the current inputs
        for ((i = 1; i <= num_runs; i++)); do
            ./exercise1_3 $thread_count $n
        done
    done
done