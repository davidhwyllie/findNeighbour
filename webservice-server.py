#!/usr/bin/env python

### SERVER
from SimpleXMLRPCServer import SimpleXMLRPCServer
import subprocess
import threading
import time
import json
import Queue

class MyXMLRPCServer(SimpleXMLRPCServer):
	"""..."""
	stopped = False

	def serve_forever(self):
			while not self.stopped:
					self.handle_request()

	@staticmethod
	def force_stop():
			MyXMLRPCServer.stopped = True
			urllib.urlopen("http://"+args.URL+"/").read()


class findNeighbour:

	def __init__(self,name="unkn",port=8184):
		##start process
		command_line = ["./findNeighbour", "--name", "{0}".format(name)]
		self.cmd=subprocess.Popen(command_line,stdout=subprocess.PIPE,stdin=subprocess.PIPE)
		self.read_semaphore = threading.BoundedSemaphore(1)
		self.write_semaphore = threading.BoundedSemaphore(1)
		##wait till system ready
		num_lines = 0
		while num_lines < 2:
                	line = self.cmd.stdout.readline()
                	if line and line != "":
				num_lines = num_lines + 1
			 	print line
		self.cmd.stdout.flush()	
	
	def insert(self,sname,dna):
                self.write_semaphore.acquire()
                self.cmd.stdin.write("INSERT {0} {1}\n".format(sname,dna))

                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
                                self.write_semaphore.release()
                                return json.loads(line)		

	def exist_sample(self,sname,dna):
		self.read_semaphore.acquire()
		self.cmd.stdin.write("EXISTSAMPLE {0}\n".format(sname))
                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
				self.read_semaphore.release()
                                return json.loads(line)

	def query_get_value(self,sname1,sname2):
		self.read_semaphore.acquire()
		self.cmd.stdin.write("GETVALUENEIGHBOUR {0} {1}\n".format(sname1,sname2))
                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
				self.read_semaphore.release()
                                return json.loads(line)
	
	def query_get_value_snp(self,sname,distance):
		self.read_semaphore.acquire()
		self.cmd.stdin.write("GETVALUE SNP {0} {1}\n".format(sname,distance))
		while True:
	  		line = self.cmd.stdout.readline()
			if line and line != "":
				self.cmd.stdout.flush()
				self.read_semaphore.release()
	  			return json.loads(line)

	def query_get_value_ids(self,sname,distance):
		self.read_semaphore.acquire()
		self.cmd.stdin.write("GETVALUE IDS {0} {1}\n".format(sname,distance))
                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
				self.read_semaphore.release()
                                return json.loads(line)

	def query_get_values_snp(self,distance):
		self.read_semaphore.acquire()
		self.cmd.stdin.write("GETALLVALUES SNP {0}\n".format(distance))
                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
				self.read_semaphore.release()
                                return json.loads(line)

	def query_get_values_ids(self,distance):
		self.read_semaphore.acquire()
		self.cmd.stdin.write("GETALLVALUES IDS {0}\n".format(distance))
                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
				self.read_semaphore.release()
                                return json.loads(line)
	
	
	def save(self):
		self.read_semaphore.acquire()
		self.write_semaphore.acquire()
		self.cmd.stdin.write("BACKUP\n")
                while True:
                        line = self.cmd.stdout.readline()
                        if line and line != "":
                                self.cmd.stdout.flush()
				self.read_semaphore.release()
				self.write_semaphore.release()
                                return json.loads(line)

	def exit(self):
		self.cmd.stdin.write("EXIT\n")
		server.stopped = 1
		return "[\"OK\"]"

if __name__=='__main__':
	import sys

	name="unkn"
	ip="localhost"
	port=int(8185)

	if len(sys.argv) > 1:	
		ip=str(sys.argv[1])
		port=int(sys.argv[2])
		name=str(sys.argv[3])

	server = MyXMLRPCServer((ip,port))
	server.register_introspection_functions()
	server.register_instance(findNeighbour(name,port))
	server.serve_forever()
