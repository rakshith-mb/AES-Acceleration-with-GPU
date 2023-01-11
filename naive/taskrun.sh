#!/usr/bin/env zsh
#SBATCH -J AES
#SBATCH -p wacc
#SBATCH -c 1
#SBATCH -t 0-0:10:00
#SBATCH -o AESSlurm.out -e AESSlurm.err

# Compile the code
g++ key_helper.cpp aes_naive.cpp main.cpp -Wall -O3 -std=c++17 -o main

# Command to run the code for default inputs
# ./main

# Command to run the code for 1024 random elements
# Comment the previous execution command and uncomment this
# Ensure that the USE_DEFAULT_INPUTS is set to 0 in main.h

./main 1024

# Command to run the timing analysis when number of elements in plain text is scaled
# Comment the previous execution command and uncomment this
# Ensure that the DISPLAY_INPUTS and USE_DEFAULT_INPUTS are set to 0 in main.h
# Change #SBATCH -t 0-0:10:00 to #SBATCH -t 0-0:30:00

# for i in {5..6}
# do 
#     val=$((2**$i))
#     echo "** Running the AES Algorithm for N = $val**"
#     ./main $val
# done