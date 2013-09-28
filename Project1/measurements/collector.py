#!/usr/bin/env python
import re

StreamTypes = [ "MMapInputStream", "MMapOutputStream", "BufferedInputStream", "BufferedOutputStream", "ReadInputStream", "WriteOutputStream", "FREADInputStream", "FWRITEOutputStream" ]
current = None
buffersize = -1

measurements = [ [] for st in StreamTypes ]

for line in open("buffered_input", 'r'):
    line = line.strip()
    if len(line) == 0 or line.startswith("Elements"):
        continue # Skip empty lines
    elif line.startswith("---"):
        current = None
        for i, val in enumerate(StreamTypes):
            if val in line:
                current = i
                break
        if current == None:
            print "INVALID STREAM TYPE!"
            print line

        numbers = re.findall(r'\d+', line)
        if len(numbers) != 2:
            buffersize = 0
        else:
            buffersize = numbers[1]
    elif line.startswith("n"):
        # Ignore
        continue
    else:
        try:
            types = (int, int, int, int, float, float, float, float, float)
            if len(line.split()) <> len(types) - 1:
                continue

            measurements[current].append( [ type(s) for type, s in zip(types, [buffersize] + line.split()) ] )
        except ValueError:
            continue # Skip invalid lines

for i,st in enumerate(StreamTypes):
    def cmp((B1, n1, k1, trials1, min1, lower1, median1, upper1, max1),
            (B2, n2, k2, trials2, min2, lower2, median2, upper2, max2)):
        if n1 <> n2:
            return n1 - n2
        if k1 <> k2:
            return k1 - k2
        if B1 <> B2:
            return B1 - B2
        return int((median1 - median2) * 1000)

    print "Measurements for " + st
    print "{0}\t{1}\t{2}\t{3}".format("B", "n", "k", "median")
    for (B, n, k, trials, min, lower, median, upper, max) in sorted(measurements[i], cmp):
        print "{0}\t{1}\t{2}\t{3}".format(B, n, k, median)
