#!/bin/bash

# Specify the range of inputs and threads to test
iteration_values=(10000 100000 1000000 10000000 100000000)
thread_values=(2 4)
approaches=(0 1)

# Specify the number of times to run the program for each input and thread count
num_runs=5


for iterations in "${iteration_values[@]}"; do
    for approach in "${approaches[@]}"; do
        for thread_count in "${thread_values[@]}"; do
            for ((i = 1; i <= num_runs; i++)); do
                ./exercise1_5 $thread_count $iterations $approach
            done
        done
    done
done