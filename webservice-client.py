#!/usr/bin/env python

### Client
import xmlrpclib
import json
import string
from __builtin__ import Exception
import socket
    
server_name = "localhost"
port = "8185"
client=xmlrpclib.ServerProxy("http://{0}:{1}".format(server_name,port))

try:
    client._()   # Call a fictive method.
except xmlrpclib.Fault:
    pass
except socket.error:
     print "ko"

print client.insert('1','ACCTGNCCTG')
print client.insert('2','ACAAGNCTCG')
print client.insert('3','ACCTGNNNAG')
print client.insert('4','ANANTNNNGG')

print client.query_get_value_snp('1','10')
print client.query_get_value('1','2')
print client.query_get_value('1','3')
print client.query_get_value('1','4')

print client.save()

print client.exit()

