from pyssandra import *
import readline
import time
import pyssandra
import uuid
from datetime import datetime
import pdb
import gzip
import os.path
import xmlrpclib
from __builtin__ import Exception
import socket
import sys

param = sys.argv[1]

server_name = sys.argv[2] 
port = sys.argv[3]
client=xmlrpclib.ServerProxy("http://{0}:{1}".format(server_name,port))

###  prepare statements
### open conection
csq=CsQuery(server="10.0.3.90")

reference_name = "R000000{0}".format(param)
reference_id = ""
reference = csq.get('reference',filterop="name={0}".format(reference_name))
reference_id = reference.fetch()[0].id

samples= open('/home/compass/samples_n{0}.txt'.format(param), 'r')
for sample_id in samples: 
	sample_id = sample_id[:-1]
	path = "/mnt/microbio/ndm-hicf/ogre/pipeline_output/{0}/MAPPING/{1}_{2}/STD/basecalls/{3}_v3.fasta.gz".format(sample_id,reference_id,reference_name,sample_id)

	if os.path.isfile(path): 
		f = gzip.open(path, 'r')

		fasta = ""
    		f.next()
    		for fastaux in f:
    			fasta += fastaux.strip()
	    	print client.insert(str(sample_id),fasta)

