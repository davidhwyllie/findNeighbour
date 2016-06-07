#!/usr/bin/env python

### Client
import xmlrpclib
import json
import string
from __builtin__ import Exception
import socket
import compassconfig
import sys
import os
from pyssandra import *
import pyssandra
import gzip
import time

### python webservice-populate-branches.py 10.0.2.57 samples_fastas_10000_assignations.txt R00000039

w = open('/home/compass/time_table_cluster.txt', 'w')
server_name = str(sys.argv[1])
fname = str(sys.argv[2])
reference_name = str(sys.argv[3])

csq=CsQuery(server="10.0.3.90")
reference = csq.get('reference',filterop="name={0}".format(reference_name))
reference_id = reference.fetch()[0].id

data = []
with open(fname, 'r') as fin:
        data = fin.read().splitlines(True)

num = 0
acc = 0
for line in data:
	line = line.replace('\n','').split('\t')
	sample_id = str(line[0]).replace('\"','')
	lineage = line[1].replace('\"','')

	branch = "{0}-{1}".format(reference_name,lineage)

	url = compassconfig.COMPASSCFG['elephantwalk'][branch.lower()]

	path = "/mnt/microbio/ndm-hicf/ogre/pipeline_output/{0}/MAPPING/{1}_{2}/STD/basecalls/{3}_v3.fasta.gz".format(sample_id,reference_id,reference_name,sample_id)
	
	client=xmlrpclib.ServerProxy(url)

	try:
	    client._()   # Call a fictive method.
	except xmlrpclib.Fault:
	    pass
	except socket.error:
	     print "ko -> ",url,lineage

        if os.path.isfile(path):
                f = gzip.open(path, 'r')

                fasta = ""
                f.next()
                for fastaux in f:
                        fasta += fastaux.strip()

		start = time.time()
                client.insert(str(sample_id),fasta)
		end = time.time()

		one = end-start
		acc = acc + one 
		num = num+1
		line = str(num)+"\t"+str(one)+"\t"+str(acc)+"\n"
		w.write(line)

		print num
	#if num > 4000:
	#	break

w.close()

