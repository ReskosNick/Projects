#!/bin/bash

# Specify the range of inputs and threads to test
m_values=(8 8000)
n_values=(8000 8000000)
p_values=(1 2 4 8 80)
thread_values=(1 2 4)

# Specify the number of times to run the program for each input and thread count
num_runs=5


for m in "${m_values[@]}"; do
    for n in "${n_values[@]}"; do
        # Skip specific combinations of m and n
        if ( [ "$m" -eq 8 ] && [ "$n" -eq 8000 ]; ) || ( [ "$m" -eq 8000 ] && [ "$n" -eq 8000000 ]; ); then
            continue
        fi
        for p in "${p_values[@]}"; do
            # Skip specific combination of m, n and p
            if [ "$m" -eq 8 ] && [ "$n" -eq 8000000 ] && [ "$p" -eq 80 ]; then
                continue
            fi
            for thread_count in "${thread_values[@]}"; do
                # Run the program multiple times for the current inputs
                for ((i = 1; i <= num_runs; i++)); do
                    ./exercise1_2 $thread_count $m $n $p
                done
            done
        done
    done
done
