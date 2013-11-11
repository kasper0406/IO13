log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./b_final.pdf"

set xlabel "log2 B"
set xtics 0, 1 rotate
set offset 1, 1

set ylabel "Running time [s]"

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", (2 ** (11+IDX)) / 1024) : sprintf("%d MB", (2 ** (11+IDX)) / (1024 * 1024))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

# TODO: Lav til MB i xticks

plot for [IDX=0:4] 'round4/b_final' i IDX using (log2($5)):($7) title mytitle(IDX+10) with linespoints