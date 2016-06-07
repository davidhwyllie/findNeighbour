#!/usr/bin/env python

### Client
import xmlrpclib
import json
import string
from __builtin__ import Exception
import socket
import sys
import os
import time

### python populate_fn_branches.py localhost R00000039

server_name = str(sys.argv[1])
reference_name = str(sys.argv[2])

branches_file = open('test/branches_config.txt', 'r')
branches = dict()
for line in branches_file:
	key = line.split('=')[0].strip()
	value = line.split('=')[1].strip() 
	branches[key]=value

print branches

samples_file = open('test/list_assign.txt', 'r')
for sample in samples_file:
        sample_lineage = sample.split('\t')[0].strip()
        sample_path = sample.split('\t')[1].strip()

	key = "{0}-{1}".format(reference_name,sample_lineage)

	url = branches[key] 
	
	client=xmlrpclib.ServerProxy(url)
	try:
	    client._()   # Call a fictive method.
	except xmlrpclib.Fault:
	    pass
	except socket.error:
	     print "ko -> ",url,sample_lineage

        if os.path.isfile(sample_path):
                f = open(sample_path, 'r')
		
		sample_id = f.next().split(' ')[0].replace('>','') 
                print sample_id
		fasta = ""
                for fastaux in f:
                        fasta += fastaux.strip()

                print client.insert(str(sample_id),fasta)
