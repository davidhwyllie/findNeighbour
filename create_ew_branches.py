#!/usr/bin/env python

import sys 
import os
import time

### need as input the IP of the machine, the reference_name, the input file with the branches id and the filename to write the config results, this output will be set up in compass.cfg file
### python create_ew_branches.py 10.0.2.57 R00000039 branches.txt branches_config.txt

ip = str(sys.argv[1])
reference_name = str(sys.argv[2])
infile = str(sys.argv[3])
outfile = str(sys.argv[4])

c = open(outfile, 'w')

data = []
with open(infile, 'r') as fin:
	data = fin.read().splitlines(True)

port = 8185
branches = []
for line in data[1:]:
	line = str(line).strip()
	line = line.split("\t")
	branches.append((line[0],port))
	c.write("{3}-{0}=http://{1}:{2} \n".format(line[0].replace('\"',''),ip,port,reference_name))
	port = port + 1

c.close()

for tup in branches:
	command = "python webservice-server.py {0} {1} {2}-{3} &".format(ip,tup[1],'/home/compass/{0}'.format(reference_name),str(tup[0]).replace('\"',''))
	os.system(command)	
	print command
