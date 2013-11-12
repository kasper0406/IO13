log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./best_sort.pdf"

set xlabel "log2 N"
set xtics 0, 1 rotate
set offset 1, 1

set ylabel "Ratio"

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2
set xrange[25:30]
set yrange[*:7]

plot 'round4/best_sort' i 1 using (log2($1)):($2/$3) title 'External heapsort (MMapStream)' with linespoints, \
     'round4/best_sort' i 2 using (log2($1)):($2/$3) title 'External heapsort (BufferedStream)' with linespoints, \
	 'round4/best_sort' i 0 using (log2($1)):($2/$3) title 'Qsort' with linespoints, \
	 'round4/best_sort' i 1 using (log2($1)):($3/$3) title 'External mergesort' with linespoints