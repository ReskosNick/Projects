#!/bin/bash

# Specify the range of inputs and threads to test
thread_values=(1 2 4)
search_percentages=(0.9 0.95 0.999)
insert_percentages=(0.05 0.025 0.0005)
approaches=(0 1 2)

# Specify the number of times to run the program for each input and thread count
num_runs=5

# Loop through thread counts
for thread_count in "${thread_values[@]}"; do
    for approach in "${approaches[@]}"; do
        # Skip combinations where approach is Serial and thread_count is not 1,
        # approach is parallel and thread_count is equal to 1
        if ( [ "$approach" -eq 0 ] && [ "$thread_count" -ne 1 ] ) || ( [ "$approach" -eq 1 ] && [ "$thread_count" -eq 1 ] ) || ( [ "$approach" -eq 2 ] && [ "$thread_count" -eq 1 ] ); then
            continue
        fi 
        # Loop through pairs of search and insert percentages
        for ((pair_index=0; pair_index<${#search_percentages[@]}; pair_index++)); do
            search_percentage=${search_percentages[$pair_index]}
            insert_percentage=${insert_percentages[$pair_index]}
            # Run the program multiple times for the current inputs
            for ((i = 1; i <= num_runs; i++)); do
                ./exercise1_4 $thread_count $search_percentage $insert_percentage $approach
            done
        done
    done
done
