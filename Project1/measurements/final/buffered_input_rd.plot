log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./buffered_input_rd.pdf"

set xlabel "k"
set xtics 0, 2, 10 rotate
set offset 1, 1
#set yrange [0:150] 
set ylabel "Running time [s]"

set ytics nomirror tc lt 1
set xtics add ("1" 0)
set xtics add ("4" 2)
set xtics add ("16" 4)
set xtics add ("64" 6)
set xtics add ("256" 8)
set xtics add ("" 9)

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

plot for [IDX=0:0] 'buffered_input' i IDX using (log2($2)):($6) title ('128kB read ahead') with linespoints, \
     'buffered_input_rd128' using (log2($2)):($6) title ('64kB read ahead') with linespoints