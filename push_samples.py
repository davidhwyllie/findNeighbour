import readline
import time
import uuid
from datetime import datetime
import pdb
import gzip
import os.path
import xmlrpclib
from __builtin__ import Exception
import socket
import sys

### push fasta files to findNeighbour
### python push_samples.py localhost R00000039


server_name = sys.argv[1] 
port = sys.argv[2]
client=xmlrpclib.ServerProxy("http://{0}:{1}".format(server_name,port))

samples= open('test/list.txt', 'r')
for sample in samples:
	sample_id = sample.split('\t')[0].strip() 
	sample_path = sample.split('\t')[1].strip()
	if os.path.isfile(sample_path): 
		f = open(sample_path, 'r')

		fasta = ""
    		f.next()
    		
		for fastaux in f:
    			fasta += fastaux.strip()
	    	print client.insert(str(sample_id),fasta)
