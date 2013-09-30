log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./fstreams.pdf"

set xlabel "k"
set xtics 0, 2, 10 rotate
set offset 1, 1

set ylabel "Running time [s]"

set ytics nomirror tc lt 1
set xtics add ("1" 0)
set xtics add ("4" 2)
set xtics add ("16" 4)
set xtics add ("64" 6)
set xtics add ("256" 8)
set xtics add ("" 9)

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

plot 'fread' using (log2($2)):($7) title "Read" with linespoints, \
     'fwrite' using (log2($2)):($7) title "Write" with linespoints
