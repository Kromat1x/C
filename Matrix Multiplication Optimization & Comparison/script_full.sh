#!/bin/bash

module load compilers/gnu-5.4.0

make clean

make

echo "-----OPT_M------"
./tema2_opt_m input
echo "Checking if ERROR < 0.001...:"
bash script_check.sh
echo "-----OPT_F------"
./tema2_opt_f input
echo "Checking if ERROR < 0.001...:"
bash script_check.sh
echo "-----BLAS-----"
./tema2_blas input
echo "Checking if ERROR < 0.001...:"
bash script_check.sh
echo "-----NEOPT-----"
./tema2_neopt input
echo "Checking if ERROR < 0.001...:"
bash script_check.sh
