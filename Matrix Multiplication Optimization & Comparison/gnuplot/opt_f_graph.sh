set term jpeg
set output "opt_f.jpeg"
set xlabel "Size"
set ylabel "Time"
set ytic 0.5
set grid

plot 'opt_f_icc.txt' with points notitle,\
'opt_f_icc.txt' with lines title 'opt_f_icc' lt rgb "#00FF00",\
'opt_f_gcc.txt' with points notitle,\
'opt_f_gcc.txt' with lines title 'opt_f_gcc' lt rgb "#FF0000";