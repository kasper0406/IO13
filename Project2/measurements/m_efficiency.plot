log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./m_efficiency.pdf"

set xlabel "Node size"
set xtics 0, 1
set offset 1, 1

set xtics add ("" 9)
set xtics add ("" 18)
set xtics add ("2 MB" 19)
set xtics add ("4 MB" 20)
set xtics add ("8 MB" 21)
set xtics add ("16 MB" 22)
set xtics add ("32 MB" 23)
set xtics add ("64 MB" 24)
set xtics add ("" 25)

set ylabel "Running time [s]"

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

plot 'round4/m_efficient' using (log2($3)):($7) title 'Efficient' with linespoints, \
     'round4/m_inefficient' using (log2($3)):($7) title 'Inefficient' with linespoints