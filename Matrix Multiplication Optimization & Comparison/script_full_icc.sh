#!/bin/bash

module load utilities/intel_parallel_studio_xe_2016

make clean

make -f Makefile.icc

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
