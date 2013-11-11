log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./m_d.pdf"

set xlabel "d"
set xtics 0, 1 rotate
set offset 1, 1

set xtics add ("" 2)
set xtics add ("8" 3)
set xtics add ("16" 4)
set xtics add ("32" 5)
set xtics add ("64" 6)
set xtics add ("128" 7)
set xtics add ("256" 8)
set xtics add ("512" 9)
set xtics add ("" 10)

set ylabel "Running time [s]"

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", 4 * (2 ** (11+2*IDX)) / 1024) : sprintf("%d MB", 4 * (2 ** (11+2*IDX)) / (1024 * 1024))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

plot 'round4/m_d' using (log2($4)):($7) title "" with linespoints