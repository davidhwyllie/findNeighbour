/**
 * @file elephant_walk.cpp
 * @name elephant_walk
 * @author oriol mazariegos
 * @date 29 April 2015
 * @brief 	Insert samples to the current database in lineal time, 1.5 seconds per 1000 samples stored in the ddbb
		Obtain related samples from a sample given a threshold 
		Obtain related samples given a threshold
 */

#include <time.h>
#include <sys/timeb.h>
#include <omp.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#define version "v2016-03-22"

using namespace std;

/****************************** FILES RECOVERY *****************************/
string name = "findNeighbour";

string samples_data = "samples_data.txt";
std::ofstream samples_data_file; 

string distance_matrix_data = "distance_matrix_data.txt";
std::ofstream distance_matrix_data_file; 

string model_data = "model_data.txt";
std::ofstream model_data_file; 

string model_ides = "model_ides.txt";
std::ofstream model_ides_file; 

string log = "log.txt";
std::ofstream log_file; 

/****************************** OPENMP THREADS *****************************/
/** @brief number threads for openmp */
int num_threads = 16;

/******************************** STRUCTURES *******************************/
/** @brief structure with the bases, vocabulary of the samples {A,C,G,T} and N  */
struct bases {
	char array[5];
};

/********************************* VARIABLES *******************************/
/** @brief max size for each sample */
unsigned long size_sample = 5000000;
/** @brief size of samples */
unsigned long num_samples = 0;
/** @brief list of samples, all in memory */
vector<string> list_samples;
/** @brief variant sites index */
vector<unsigned long> indexes;
/** @brief bases + N */
string bases_sim [5] = {"A","C","T","G","N"};
/** @brief ref model for invariant sites and N's */
string model;
/** @brief distance matrix result */
vector<vector<unsigned long> > matrix;
/** @brief ides structures linkage name with index*/
unordered_map<string,unsigned long> ides_name_index;
/** @brief ides structures linkage index with name*/
unordered_map<unsigned long,string> ides_index_name;
/** @brief ides structures for processed samples*/
unordered_set<string> ides_name;
/** @brief API keys */
unordered_map<string,unsigned long> APIID {   { "INSERT", 0 }
			                    , { "EXIT", 6 }
		                            , { "EXISTSAMPLE", 5 }
	                                    , { "GETVALUENEIGHBOUR", 4 }
                                            , { "GETVALUE", 1 }
                                            , { "GETALLVALUES", 3 }
                                            , { "BACKUP", 2 }					
					    , { "GETMODEL", 7 } 
};

unordered_map<string,unsigned long> APIVALUEID 	{ { "SNP", 0 }
                                          	, { "IDS", 1 } };

/**************************** AUX FUNCTIONS ***************************/
const string current_date_time() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

int file_exist (char *filename)
{
    char s[200];
    sprintf(s, "test -e %s", filename);
    if (system(s) == 0){
        return 1;
    }else{
        return 0;
    }
}

/**************************** READ/WRITE ***************************/
void write_log(string message){
	log_file.open(log, std::ios::app); 
	log_file << message << endl;
	log_file.close(); 	
}

void write_sample_file(string id, string * sample){
	timeb first_uniq;
	ftime(&first_uniq);

	//if(recovery == 1) return;
	samples_data_file.open(samples_data,std::ios::app); 
	//samples_data_file << ">" << id << "\n"<< sample << endl; 
	samples_data_file << "INSERT" << "\t" << id << "\t" << *sample << endl; 
	samples_data_file.close(); 


	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	
	write_log("{action:process_sample, id:"+id+", time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void write_samples_file(){

	timeb first_uniq;
	ftime(&first_uniq);

	samples_data_file.open(samples_data, std::ofstream::out | std::ofstream::trunc);

	for(unsigned long i=0; i<num_samples; i++){
		string id = ides_index_name[i];
		string sample = list_samples[i];
		samples_data_file << "INSERT" << "\t" << id << "\t" << sample << endl; 
	}

	samples_data_file.close(); 

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:process_sample, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void write_sample_file(){

	timeb first_uniq;
	ftime(&first_uniq);

	samples_data_file.open(samples_data, ios::app); //open at the end

	unsigned long i=num_samples-1;
	string id = ides_index_name[i];
	string sample = list_samples[i];
	samples_data_file << "INSERT" << "\t" << id << "\t" << sample << endl; 
	
	samples_data_file.close(); 

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:process_sample, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}


void recovery_samples_file(){
	ifstream infile(samples_data);

	list_samples.resize(num_samples);

	//read all file
	string line;
	string INSERT;
	string id;
	string sample;
	while (getline(infile, line)) {
		istringstream iss(line);
		iss >> INSERT >> id >> sample;

		//add sample to the list
		unsigned long index = ides_name_index[id];
		list_samples[index] = sample;
	}


	infile.close(); 
}

void write_model(){
	model_data_file.open(model_data, std::ofstream::out | std::ofstream::trunc); 
	model_data_file.write( model.c_str(), sizeof(char)*size_sample );
	model_data_file << endl;
	model_data_file.close(); 
}

void recovery_model(){
	ifstream infile(model_data);

	getline(infile, model);

	size_sample = model.length();

	infile.close(); 
}

void delete_sample_file(){
	samples_data_file.open(samples_data, std::ofstream::out | std::ofstream::trunc);
	samples_data_file.close();
}

void write_distance_matrix(){
	distance_matrix_data_file.open(distance_matrix_data, std::ofstream::out | std::ofstream::trunc); 

	for(unsigned long i=0; i<num_samples; i++){
		string id1 = ides_index_name[i];
		for(unsigned long j=i+1; j<num_samples; j++){
			string id2 = ides_index_name[j];
			unsigned long distance = matrix[i][j];	
			distance_matrix_data_file << id1 << "\t" << id2 << "\t" << distance << endl; 
		}	
	}

	distance_matrix_data_file.close(); 
}

void write_distance_matrix_sample(string name){
	distance_matrix_data_file.open(distance_matrix_data, ios::app); 

	unordered_map<string,unsigned long>::const_iterator it_ides_name_index;
	it_ides_name_index = ides_name_index.find(name);

	unsigned long i=it_ides_name_index->second;
	string id1 = ides_index_name[i];
	for(unsigned long j=0; j<i; j++){
		string id2 = ides_index_name[j];
		unsigned long distance = matrix[i][j];	
		distance_matrix_data_file << id1 << "\t" << id2 << "\t" << distance << endl; 
	}	

	distance_matrix_data_file.close(); 
}

void recovery_distance_matrix(){
	ifstream infile(distance_matrix_data);

	//read first line with the total number of samples
	unordered_map<string,string> aux;
	string line;
	string id1;
	string id2;
	unsigned long value ;

	matrix.resize( num_samples , vector<unsigned long>( num_samples , 0 ) );

	//read all file
	while (getline(infile, line)) {
		istringstream iss(line);
		iss >> id1 >> id2 >> value;

		unsigned long index_1 = ides_name_index[id1];
		unsigned long index_2 = ides_name_index[id2];

		matrix[index_1][index_2] = value;
		matrix[index_2][index_1] = value;
	}

	infile.close(); 
}

void write_ides(){
	model_ides_file.open(model_ides, std::ofstream::out | std::ofstream::trunc); 

	for(unsigned long i=0; i<ides_index_name.size(); i++){
		string name = ides_index_name[i];
		model_ides_file << i << "\t" << name << endl; 
	}

	model_ides_file.close(); 
}

void write_id(string name){
	model_ides_file.open(model_ides, ios::app); 

	//unsigned long i=ides_index_name.size()-1;
	//string name = ides_index_name[i];
	unordered_map<string,unsigned long>::const_iterator it_ides_name_index;
	it_ides_name_index = ides_name_index.find(name);
	unsigned long i=it_ides_name_index->second;
	
	model_ides_file << i << "\t" << name << endl; 
	
	model_ides_file.close(); 
}

void recovery_ides(){
	ifstream infile(model_ides);

	unsigned long index;
	string name;
	string line;
	while (getline(infile, line)) {
		istringstream iss(line);
		iss >> index >> name;
	
		ides_index_name[index] = name;
		ides_name_index[name] = index;
		ides_name.insert(name);
	}

	num_samples = ides_index_name.size();

	infile.close(); 
}

/****************************** DISTANCE MATRIX *****************************/
int compare_fast(char a, char b) {

	switch(a) {
		case 'a': a = 'A'; break;
		case 'c': a = 'C'; break;
		case 't': a = 'T'; break;
		case 'g': a = 'G'; break;
	}

	switch(b) {
		case 'a': b = 'A'; break;
		case 'c': b = 'C'; break;
		case 't': b = 'T'; break;
		case 'g': b = 'G'; break;
	}

	bool noerr = 1;
	switch(a) {
		case 'U': case 'u': a = 'T'; break;
		case ' ': case '\t': case '\r': case '\n': case '\v': case '\b': case '\f': noerr = 0; break;
	}
	switch(b) {
		case 'U': case 'u': b = 'T'; break;
		case ' ': case '\t': case '\r': case '\n': case '\v': case '\b': case '\f': noerr = 0; break;
	}
	return (a!=b) && !(a=='N' || b=='N') && !(a=='-' || b=='-') && !(a=='?' || b=='?') && noerr;
}

void process_sample(){

	timeb first_uniq;
	ftime(&first_uniq);

	unsigned long N = list_samples.size();
	unsigned long i = N-1;
	#pragma omp parallel for num_threads(num_threads)
	for(int j=0; j<N-1; j++){
		unsigned long value = 0;
		for(unsigned long k=0; k<indexes.size(); k++){
			unsigned long z = indexes[k];
			value += compare_fast(list_samples[i][z],list_samples[j][z]);
		}
		matrix[i][j] = value;
		matrix[j][i] = value;
	}

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:process_sample, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void clean_data_from_model(){

	indexes.clear();

	for(int i=0; i<size_sample; i++){
		if(model[i] == '.'){
			indexes.push_back(i);
		}
	}
}

/****************************** MODEL *****************************/
void init_model(){
	for(unsigned long j=0; j<size_sample; j++){
		model.append("\t");
	}
}

void change_model(string * sample){
	timeb first_uniq;
	ftime(&first_uniq);

	for(unsigned long i=0; i<size_sample; i++){
		if(model[i] == '\t') {
			model[i] = (*sample)[i];
		}else{
			if(model[i] != '.'){
				if(model[i] == 'N' || model[i] == '-'){
					if(((*sample)[i] != 'N' && (*sample)[i] != '-')){
						model[i] = (*sample)[i];
					}
				}else if(model[i] != (*sample)[i] && (*sample)[i]!='N' && (*sample)[i]!='-'){
					model[i] = '.';
				}				
			}
		}
	}

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:change_model, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}


/****************************** SAMPLE MATRIX *****************************/

void add_sample_to_matrix(){

	timeb first_uniq;
	ftime(&first_uniq);

	unsigned long size_fila = matrix.size();
	unsigned long size_columna = 0;
	if(size_fila > 0){
		size_columna = matrix[0].size();
	}

	vector<unsigned long> row(size_columna+1);

	for(unsigned long i; i<size_columna+1; i++){
		row[0] = 0;	
	}

	for(unsigned long i; i<size_fila; i++){
		matrix[i].push_back(0);	
	}
	
	matrix.push_back(row);

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:add_sample_to_matrix, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

bool exist_sample(string id){
	timeb first_uniq;
	ftime(&first_uniq);

	bool exist = true;

	unordered_map<string,unsigned long>::const_iterator it_ides_name_index;
	it_ides_name_index = ides_name_index.find(id);
		
	if ( it_ides_name_index == ides_name_index.end()){
		return false;
	}

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:exist_sample, id:"+id+", time:"+std::to_string(dif)+", date:"+current_date_time()+"}");

	return exist;
}

bool processed_sample(string id){
	timeb first_uniq;
	ftime(&first_uniq);

	bool exist = true;

	unordered_set<std::string>::const_iterator it_ides;
	it_ides = ides_name.find(id);
		
	if ( it_ides == ides_name.end()){
		return false;
	}

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:processed_sample, id:"+id+", time:"+std::to_string(dif)+", date:"+current_date_time()+"}");

	return exist;
}

/****************************** API *****************************/

string add_sample(string id, string * sample){
	timeb first_uniq;
	ftime(&first_uniq);

	//check the first insertion and fix the length
	if(num_samples == 0){
		size_sample = (*sample).length();
	}else{
		if(size_sample!=(*sample).length()){
			return "['Err','missmatch length sample']";
		}
	}

	//add sample to the list
	list_samples.push_back(*sample);

	ides_index_name[num_samples] = id;
	ides_name_index[id] = num_samples;

	num_samples++;
	size_sample = (*sample).length();
	
	add_sample_to_matrix();

	//clean data
	change_model(sample);
	clean_data_from_model();

	//process distance
	process_sample();

	//insert to processed samples
	ides_name.insert (id); 
	 
	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:add_sample, id:"+id+", time:"+std::to_string(dif)+", date:"+current_date_time()+"}");

	return "[\"OK\"]";
}

void get_values_neighbour(string id, string idrelated){
	timeb first_uniq;
	ftime(&first_uniq);

	string snps = "[\"OK\",";
	unsigned long index = ides_name_index.at(id);
	unsigned long indexrelated = ides_name_index.at(idrelated);

	snps.append(to_string(matrix[index][indexrelated]));
	snps.append("]");

	cout << snps << endl;

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:get_value_neighbour, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void get_value_snps(string id, unsigned long threshold){
	timeb first_uniq;
	ftime(&first_uniq);

	string snps = "[\"OK\",[";
	unsigned long index = ides_name_index.at(id);
	unsigned long related_samples = 0;

	for(unsigned long i=0; i<num_samples; i++){
		if(matrix[index][i] <= threshold && i!=index){
			snps.append("[");
			snps.append("\""+ides_index_name.at(i)+"\"");
			snps.append(","+to_string(matrix[index][i]));
			snps.append("],");
			related_samples++;
		}
	}
	if(snps.length() > 0 && related_samples>0) snps.pop_back();

	snps.append("]]");

	cout << snps << endl;

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:get_value, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void get_value_ides(string id, unsigned long threshold){
	timeb first_uniq;
	ftime(&first_uniq);

	string snps = "[\"OK\",[";
	unsigned long index = ides_name_index.at(id);
	unsigned long related_samples = 0;
	for(unsigned long i=0; i<num_samples; i++){
		if(matrix[index][i] <= threshold && i!=index){
			snps.append("\""+ides_index_name.at(i)+"\"").append(",");
			related_samples++;
		}
	}
	//f(snps.length() > 0) snps.pop_back();
	if(snps.length() > 0 && related_samples>0) snps.pop_back();

	snps.append("]]");

	cout << snps << endl;

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:get_value, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void get_all_values_snps(unsigned long threshold){

	timeb first_uniq;
	ftime(&first_uniq);

	string snps = "[\"OK\",[";
	unsigned long related_samples = 0;
	for(unsigned long i=0; i<num_samples; i++){
		for(unsigned long j=i+1; j<num_samples; j++){
			if(matrix[i][j] <= threshold){
				snps.append("[");
				snps.append("\""+ides_index_name.at(i)+"\"").append(",");
				snps.append("\""+ides_index_name.at(j)+"\"").append(",");
				snps.append(std::to_string(matrix[i][j]));
				snps.append("],");
				related_samples++;
			}
		}
	}	

	if(snps.length() > 0 && related_samples>0) snps.pop_back();

	snps.append("]]");

	cout << snps << endl;

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:get_all_values_snps, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

void get_all_values_ides(unsigned long threshold){

	timeb first_uniq;
	ftime(&first_uniq);

	string snps = "[\"OK\",[";

	char *aux = (char *)malloc(num_samples*sizeof(char));
	memset(aux, '0', num_samples*sizeof(char));

	for(unsigned long i=0; i<num_samples; i++){
		for(unsigned long j=i+1; j<num_samples; j++){

			if(matrix[i][j] <= threshold){
				
				if(aux[i] == '0') {aux[i] = '1';}
				if(aux[j] == '0') {aux[j] = '1';}
			}
		}
	}	

	unsigned long related_samples = 0;
	for(unsigned long i=0; i<num_samples; i++){
		if(aux[i] == '1') {
			snps.append("\""+ides_index_name.at(i)+"\"").append(",");
			related_samples++;
		}
	}

	if(snps.length() > 0 && related_samples>0) snps.pop_back();

	free(aux);

	snps.append("]]");

	cout << snps << endl;	

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:get_all_values_ides, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
}

string get_model(){
	if (num_samples == 0){
		return "";
	}else{
		return model;
	}
}

/****************************** PRINTS *****************************/

void print_samples(){
	for(unsigned long i=0; i<list_samples.size(); i++){	
		cout << list_samples[i] << endl;;
	}
}

void print_samples_indexes(){
	for(unsigned long i=0; i<list_samples.size(); i++){	
		for(unsigned long k=0; k<indexes.size(); k++){
			unsigned long z = indexes[k];
			cout << list_samples[i][z];
		}
		cout << endl;
	}
}

void print_indexes(){
	for(unsigned long i=0; i<indexes.size(); i++){	
		cout << indexes[i] << "|";
	}
	cout << endl;
}

void print_matrix(){
	for(unsigned long i=0; i<matrix.size(); i++){
		for(unsigned long j=0; j<matrix[i].size()-1; j++){		
			cout << matrix[i][j] << "|";
		}
		cout << matrix[i][matrix[i].size()-1] << endl;
	}
}

void print_model(){
	for(unsigned long i=0; i<size_sample; i++){	
			cout << model[i];
	}
	cout << endl;
}

void print_snps(){
	//cout << snps << endl;
}

/****************************** TESTS *****************************/

void test_process_samples(){
	unsigned long N = list_samples.size();
  	#pragma omp parallel for num_threads(num_threads)
	for(unsigned long i=0; i<N; i++){
		for(unsigned long j=i+1; j<N; j++){
			unsigned long value = 0;
			for(unsigned long k=0; k<indexes.size(); k++){
				unsigned long z = indexes[k];
				value += compare_fast(list_samples[i][z],list_samples[j][z]);
			}
			matrix[i][j] = value;
			matrix[j][i] = value;
		}
	}
}

void test_process_sample(){

	string sample1 = "";
	string sample2 = "";
	for(int i=0; i<4000000; i++){
		sample1.append("A");	
		sample2.append("T");	
	}

	indexes.clear();
	for(int i=0; i<4000000; i++){
		unsigned long v = rand() % 15;	
		if(v == 1) indexes.push_back(i);
	}

	vector<int> result(20000);

	timeb first_uniq;
	ftime(&first_uniq);

  	#pragma omp parallel for num_threads(num_threads)
	for(int i=0; i<20000; i++){
		long int value = 0;
		for(unsigned long k=0; k<indexes.size(); k++){
			unsigned long z = indexes[k];
			value += compare_fast(sample1[z],sample2[z]);
		}	
		result[i] = value;
	}

	timeb end_first_uniq;
	ftime(&end_first_uniq);
	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	cout << "TIME 1 SAMPLE" << dif << " VALUE " << result[0] << " SIZE INDEXES" << indexes.size() << endl;
}

/****************************** HELP *****************************/
void print_help(){

	printf ("FINDNEIGHBOUR application HELP - %s\n",version);

	printf("\n");

	printf ("Options:\n");
	printf("  --h, --help,				show this help message and exit\n");
	printf("  --t, --threads numthreads, 		number of threads for openmp library\n");

	printf("\n");

	printf("Use starting:\n");
	printf("  > ./name_app --threads 8 \n");
	printf("  > ./name_app -t 8 \n");
	printf("  > ./name_app\n");

	printf("\n");

	printf("Use example:\n");
	printf(" input  > INSERT id_sample sample_fasta\n");
	printf(" output > OK|Err\n");
	printf(" input  > GETVALUE [ids|snp] id_sample threshold\n");
	printf(" output > Err| ids -> list_of_id_samples [\"id_sample1\",..,\"id_sampleN\"]\n");
	printf("               snp -> list_of_id_samples_with_snp [\"id_sample1\",snp],..,[\"id_sampleN\",snpN]]\n");

	printf(" input -> GETALLVALUES [ids|snp] threshold\n");
	printf(" output -> Err| ids -> list_of_id_samples [\"id_sample1\",..,\"id_sampleN\"]\n");
	printf("                snp -> list_of_id_samples_pair_with_snp [\"id_sample1\",\"id_sample2\",snp1-2]..[\"id_sampleN\",\"id_sampleM\",snpN-M]]\n");

}
/****************************** INIT *****************************/
void save_system(){

	timeb first_uniq;
	ftime(&first_uniq);

	//save samples
	write_samples_file();

	//save distance matrix
	write_distance_matrix();

	//save model
	write_model();

	//save ides
	write_ides();

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:save_system, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");

}

void save_sample(string id, string * sample){

	timeb first_uniq;
	ftime(&first_uniq);

	//save sample
	write_sample_file(id,sample);

	//save distance matrix
	write_distance_matrix_sample(id);

	//save model
	write_model();

	//save ides
	write_id(id);

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:save_sample, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");

}

void setup_path_files(){
	
	samples_data = name+"/"+samples_data;
	distance_matrix_data = name+"/"+distance_matrix_data;
	model_data = name+"/"+model_data;
	model_ides = name+"/"+model_ides;
	log = name+"/"+log;

	//create folder for name if doesn't exist
	char *cstr;
	cstr = new char[name.length() + 1];
	strcpy(cstr, name.c_str());
	if (file_exist(cstr) == 0){
		mkdir(name.c_str(), 0700);
	}
	
}

void init_system(){

	cout << "Initializing system..." << endl;

	timeb first_uniq;
	ftime(&first_uniq);

	//init backup folder
	setup_path_files();
	
	//init num samples
	num_samples = 0;
	//init model
	init_model();
	write_log("{action:init_model, date:"+current_date_time()+"}");
	//recovery ides
	recovery_ides();
	write_log("{action:recovery_ides, date:"+current_date_time()+"}");
	//recovery samples
	recovery_samples_file();
	write_log("{action:recovery_samples_file, date:"+current_date_time()+"}");
	//recovery model
	recovery_model();
	write_log("{action:recovery_model, date:"+current_date_time()+"}");
	//recovery distance matrix
	recovery_distance_matrix();
	write_log("{action:recovery_distance_matrix, date:"+current_date_time()+"}");

	timeb end_first_uniq;
	ftime(&end_first_uniq);

	int dif = (int)((end_first_uniq.time*1000+end_first_uniq.millitm)-(first_uniq.time*1000+first_uniq.millitm));
	write_log("{action:init_system, time:"+std::to_string(dif)+", date:"+current_date_time()+"}");
	
	cout << "System ready !!" << endl;
}

/****************************** MAIN *****************************/
int main (int argc, char **argv)
{
	int c;
	while (1) {
		static struct option long_options[] =
		{
			//These options don’t set a flag.
			//We distinguish them by their indices. 
			{"help",   	no_argument, 		0, 		'h'},
			{"threads",   	required_argument, 	0, 		't'},
			{"name",   	required_argument, 	0, 		'n'},
			{0, 		0, 			0, 		0}
		};
		//getopt_long stores the option index here. 
		int option_index = 0;

		c = getopt_long (argc, argv, "h:t:o:p:r:",long_options, &option_index);

		//Detect the end of the options.
		if (c == -1) { 
			break;
		}

		switch (c) {
			case 'h':
				print_help();
				return 0;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'n':
				name = optarg;
				break;
		}
		
	}

	init_system();

	string aux_model;
	string id;
	string idrelated;
	string sample;
	int threshold;
	string func = "";;
	string line = "";
	int func_id;
	unordered_map<string,unsigned long>::const_iterator it_func_id;
	unordered_map<string,unsigned long>::const_iterator it_func_value_id;
	while (getline(std::cin, line)) {
		if(!line.empty()){
			istringstream iss(line);

			iss >> func >> id;

			it_func_id = APIID.find(func);

			if (it_func_id != APIID.end()){

				switch(it_func_id->second) {	
					case 0:
						if(exist_sample(id)) {
							cout << "[\"Err\",\"duplicated sample\"]" << endl;
						}else{
							iss >> sample;
							string ok = add_sample(id,&sample);
							
							save_sample(id,&sample);
							
							cout << ok << endl;
						}
						break;
					case 1:
						it_func_value_id = APIVALUEID.find(id);
						if(it_func_value_id == APIVALUEID.end()){
							cout << "[\"Err\",\"missmatch function\"]" << endl;
						}else{
							iss >> id;
							if(!exist_sample(id)) {
								cout << "[\"Err\",\"missing sample\"]" << endl;
							}else{
								iss >> threshold;
								switch(it_func_value_id->second) {	
									case 0: get_value_snps(id,threshold);
										break;
									case 1: get_value_ides(id,threshold);
										break;
								}
							}
						}

						break;
					case 3:
						it_func_value_id = APIVALUEID.find(id);
						if(it_func_value_id == APIVALUEID.end()){
							cout << "[\"Err\",\"missmatch function\"]" << endl;
						}else{
							iss >> threshold;
							switch(it_func_value_id->second) {	
								case 0: get_all_values_snps(threshold);
									break;
								case 1: get_all_values_ides(threshold);
									break;
							}
						}

						break;
					case 2:
						save_system();
						cout << "[\"OK\"]" << endl;
						break;
					case 4:
						iss >> idrelated;
						if(!exist_sample(id) || !exist_sample(idrelated)) {
							cout << "[\"Err\",\"missing sample\"]" << endl;
						}else{
							get_values_neighbour(id,idrelated);
						}
						break;
					case 5:
						iss >> idrelated;
						if(!processed_sample(id)) {
							cout << "[\"Err\",\"missing sample\"]" << endl;
						}else{
							cout << "[\"OK\"]" << endl;
						}
						break;
					case 6:
						write_log("{action:exit_system, date:"+current_date_time()+"}");
						cout << "Bye !!" << endl;
						return 0;
						break;
					case 7:
						aux_model = get_model();
						cout << "[\"OK\",\"" << aux_model << "\"]" << endl;
						break;
					default: 
						cout << "[\"Err\",\"missing function\"]" << endl;
						break;
				}
			}else{
				cout << "[\"Err\",\"missmatch function\"]" << endl;
			}
		}
	}

	return 0;
}
