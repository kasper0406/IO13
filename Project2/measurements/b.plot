log2(x) = log(x)/log(2)
max(a,b) = (a > b) ? a : b

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./b_final.pdf"

set xlabel "Buffer size"
#set xtics 0, 1 rotate
set offset 1, 1
set yrange [*:1000]

set ytics nomirror tc lt 1
set xtics add ("1" 0)
set xtics add ("4" 2)
set xtics add ("16" 4)
set xtics add ("64" 6)
set xtics add ("256" 8)
set xtics add ("" 9)
set xtics add ("" 10)
set xtics add ("8 kB" 11)
set xtics add ("16 kB" 12)
set xtics add ("32 kB" 13)
set xtics add ("64 kB" 14)
set xtics add ("128 kB" 15)
set xtics add ("" 16)

set ylabel "Running time [s]"

set ytics nomirror tc lt 1

mytitle(IDX) = (IDX < 5) ? sprintf("%d kB", (2 ** (11+IDX)) / 1024) : sprintf("%d MB, d = %d", (2 ** (11+IDX)) / (1024 * 1024), 268435456 / 2**(IDX+9))

#set logscale y
set grid mytics

set key vert left top Left reverse
set pointsize 2

# TODO: Lav til MB i xticks

plot for [IDX=0:4] 'round4/b_final' i IDX using (log2($5)):($7) title mytitle(IDX+10) with linespoints