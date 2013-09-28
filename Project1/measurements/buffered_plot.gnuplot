#!/usr/bin/env gnuplot
log2(x) = log(x) / log(2)

set terminal pdf enhanced font "Helvetica, 10" size 7,4
set output "./buffered_plot.pdf"

set xlabel "B"
set ylabel "k"
set zlabel "Time"

set logscale x
set logscale y

unset key

set dgrid3d 100, 100
set hidden3d
#set isosamples 10

splot 'data.dat' using ($1):($3):($4) with lines
