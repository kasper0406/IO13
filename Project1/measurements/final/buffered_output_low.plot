log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./buffered_output_low.pdf"

set xlabel "k"
set xtics 0, 1 rotate
set offset 1, 1

set ylabel "Running time [s]"

set ytics nomirror tc lt 1
set xtics add ("1" 0)
set xtics add ("2" 1)
set xtics add ("4" 2)
set xtics add ("8" 3)
set xtics add ("16" 4)
set xtics add ("32" 5)
set xtics add ("64" 6)
set xtics add ("128" 7)
set xtics add ("256" 8)
set xtics add ("" 9)

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

plot for [IDX=0:3] 'buffered_output' i IDX using (log2($2)):($6) title mytitle(IDX) with linespoints