set term jpeg
set output "icc.jpeg"
set xlabel "Size"
set ylabel "Time"
set ytic 2.5
set grid

plot 'neopt_icc.txt' with points notitle,\
'neopt_icc.txt' with lines title 'neopt' lt rgb "#00FF00",\
'opt_m_icc.txt' with points notitle,\
'opt_m_icc.txt' with lines title 'opt_m' lt rgb "#FF0000",\
'opt_f_icc.txt' with points notitle,\
'opt_f_icc.txt' with lines title 'opt_f' lt rgb "#0000FF",\
'blas_icc.txt' with points notitle,\
'blas_icc.txt' with lines title 'blas' lt rgb "#FF00FF";