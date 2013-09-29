log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./best_input.pdf"

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

plot for [IDX=4:4] 'buffered_input' i IDX using (log2($2)):($7) title mytitle(IDX) . ' Buffered' with linespoints, \
     for [IDX=5:5] 'mmap_read' i IDX using (log2($2)):($7) title mytitle(IDX) . ' MMAP' with linespoints, \
     'read' using (log2($2)):($7) title 'read' with linespoints, \
     'fread' using (log2($2)):($7) title 'fread' with linespoints