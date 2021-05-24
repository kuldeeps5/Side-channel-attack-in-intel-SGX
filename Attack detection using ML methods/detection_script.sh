#!/usr/bin/bash
# sudo sh -c 'echo -1 >/proc/sys/kernel/perf_event_paranoid'

rm -f *data.txt
rm -f *data.csv

sleeptime=60 # time in seconds. Supports decimals

perf stat -aA -I 1 --append -o a_data.txt -e branches,cache-misses,cache-references -x , sleep $sleeptime

python3 formatter.py
