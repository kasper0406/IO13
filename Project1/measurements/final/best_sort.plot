log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./best_sort.pdf"

set xlabel "log2 k"
set xtics 0, 1 rotate
set offset 1, 1

set ylabel "Running time [s]"

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

plot 'qsort' every ::22 using (log2($1)):($5) title 'Quicksort' with linespoints, \
     'heapsort' every ::22 using (log2($1)):($5) title 'Heapsort' with linespoints
     # TODO(lespeholt): Mangler den bedste external merge for alle punkterne