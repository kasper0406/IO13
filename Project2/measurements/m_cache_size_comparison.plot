log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./m_cache_size_comparison.pdf"

set xlabel "Node size"
#set xtics 0, 1 rotate
set offset 1, 1

set ylabel "Running time [s]"

set xtics add ("" 9)
set xtics add ("" 18)
set xtics add ("2 MB" 19)
set xtics add ("4 MB" 20)
set xtics add ("8 MB" 21)
set xtics add ("16 MB" 22)
set xtics add ("" 23)

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

set grid mytics

set key vert left top Left reverse
set pointsize 2

plot 'm_cache_size_comparison' i 0 using (log2($3)):($7) title 'No cache' with linespoints, \
     'm_cache_size_comparison' i 1 using (log2($3)):($7) title '4 kB cache' with linespoints, \
     'm_cache_size_comparison' i 2 using (log2($3)):($7) title '8 kB cache' with linespoints, \
     'm_cache_size_comparison' i 3 using (log2($3)):($7) title '16 kB cache' with linespoints, \
     'm_cache_size_comparison' i 4 using (log2($3)):($7) title '32 kB cache' with linespoints