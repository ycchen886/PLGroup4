#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: <input_file> <output_file>" >&2
  exit 1
fi

infile="$1"
tmpfile1=$infile".tmp.1"
tmpfile2=$infile".tmp.2"
outfile="$2"

# Need to remove n lines at the beginning and at the end of the file (non-content part)

# remove the line: [Illustration ......]
perl -0777 -p -e 's|\n\n\[Illustration.*\]\n\n| |g' $infile > $tmpfile1

# append the indented paragraph to its previous one
perl -0777 -p -e 's|(\n\n) +| |g' $tmpfile1 > $tmpfile2

# add <s>, <m>, <e> tag
./preprocess $tmpfile2 $outfile

rm $tmpfile1 $tmpfile2


