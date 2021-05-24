import os, sys
import numpy as np 
import csv
import fnmatch, subprocess

from collections import defaultdict

for root, dir, files in os.walk("."):
	print(root[2:])
	print("")
	for items in fnmatch.filter(files, "*data.txt"):
		d = defaultdict(list)
		#print(items)
		val = np.loadtxt(items, dtype='str',delimiter=',',usecols=(2,4))
		#print(val)
		for j in range(len(val)):
			if val[j][0] == '<not supported>' or val[j][0] == '<not counted>' :
				# print(val[j][1])
				continue
			d[val[j][1]].append(int(val[j][0]))
		# for key, value in d.items() :
		# 	print(len(value))
		with open(items[:-4] + ".csv", "w") as outfile:
			writer = csv.writer(outfile)
			writer.writerow(d.keys())
			writer.writerows(zip(*d.values()))
	print("")
