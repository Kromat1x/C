set term jpeg
set output "blas.jpeg"
set xlabel "Size"
set ylabel "Time"
set ytic 0.25
set grid

plot 'blas_icc.txt' with points notitle,\
'blas_icc.txt' with lines title 'blas_icc' lt rgb "#00FF00",\
'blas_gcc.txt' with points notitle,\
'blas_gcc.txt' with lines title 'blas_gcc' lt rgb "#FF0000";