Abstract:
Insert samples to the current database in real time, less than 10 seconds
Grasp related samples from a sample given a threshold 


Compile:
>make clean
>make

or

>g++ -std=c++11 -fopenmp -O3 open_elephant_walk.cpp -lz -o name_app


Examples of execution:

./name_app --threads 8 
./name_app -t 8 
./name_app

by default the value of threads is 8 and recovery is 0

-threads is the variable to determine the numbe rof threads running in the multithreadhing system
-recovery determine if the system starts like a first time (0) or recover with the data saved in samples_data.fasta

Examples of services

input -> INSERT id_sample fasta_sample
output -> OK|Err

input -> GETVALUE [ids|snp] id_sample threshold
output -> Err| 	ids -> list_of_id_samples ['id_sample1',..,'id_sampleN']
		snp -> list_of_id_samples_with_snp ['id_sample1',snp],..,['id_sampleN',snpN]]

input -> GETALLVALUES [ids|snp] threshold 
output -> Err| 	ids -> list_of_id_samples ['id_sample1',..,'id_sampleN']
		snp -> list_of_id_samples_pair_with_snp ['id_sample1','id_sample2',snp1-2]..[id_sampleN id_sampleM snpN-M]]

input -> BACKUP
output -> OK|Err

INSERT 1 ACCTGNCCTG
INSERT 2 ACAAGNCTCG
INSERT 3 ACCTGNNNAG
INSERT 4 ANANTNNNGG

GETVALUE SNP 1 10
GETVALUE IDS 1 10
GETALLVALUES SNP 10
GETALLVALUES IDS 10

BACKUP

Examples of execution webservice:

server -> 
python webservice-server.py

client->
python webservice-client.py

cat webservice-client.py
### Client
import xmlrpclib

client=xmlrpclib.ServerProxy("http://localhost:8184")

print client.insert('1','ACCTGNCCTG')
print client.insert('2','ACAAGNCTCG')
print client.insert('3','ACCTGNNNAG')
print client.insert('4','ANANTNNNGG')

print client.query_get_value_ids('1','5')
print client.query_get_value_snp('1','5')
print client.query_get_values_ids('5')
print client.query_get_values_snp('5')

print client.save()

###execution service
python webservice_server.py <ip> <port> <path_to_store_files>
ex: 
python webservice_server.py 10.0.2.54 8185 /home/compass/R00000039

###execute update database
python get_samples.bh <ip> <port> <refname>
ex:
python get_samples.bh 10.0.2.54 8185 039

###execute multiple instances of findNeighbour
python create_ew_branches.py <IP> <ref_id> <input_file> <output_file>
ex:
python create_ew_branches.py 10.0.2.57 R00000039 branches.txt branches_config.txt

###execute update database
python webservice-populate-branches.py <IP> <input_file> <ref_id>
ex:
python webservice-populate-branches.py 10.0.2.57 samples_fastas_10000_assignations.txt R00000039
