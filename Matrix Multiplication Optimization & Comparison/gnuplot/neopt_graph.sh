set term jpeg
set output "neopt.jpeg"
set xlabel "Size"
set ylabel "Time"
set ytic 2.5
set grid

plot 'neopt_icc.txt' with points notitle,\
'neopt_icc.txt' with lines title 'neopt_icc' lt rgb "#00FF00",\
'neopt_gcc.txt' with points notitle,\
'neopt_gcc.txt' with lines title 'neopt_gcc' lt rgb "#FF0000";