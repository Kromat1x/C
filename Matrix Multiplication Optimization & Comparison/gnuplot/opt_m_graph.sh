set term jpeg
set output "opt_m.jpeg"
set xlabel "Size"
set ylabel "Time"
set ytic 1.5
set grid

plot 'opt_m_icc.txt' with points notitle,\
'opt_m_icc.txt' with lines title 'opt_m_icc' lt rgb "#00FF00",\
'opt_m_gcc.txt' with points notitle,\
'opt_m_gcc.txt' with lines title 'opt_m_gcc' lt rgb "#FF0000";