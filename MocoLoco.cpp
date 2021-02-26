#include "MocoLoco.h"

int main(int argc, char *argv[]){


	if(argc == 1){             //If arguments number is 1 means that no input file has been inserted - display help
		display_help();
	}

	command_line_parser(argc, argv);					//Parser function called to handle aguments
	vector<bed_class> GEP;					//Initializing GEP --> vector of bed_class classes
	vector<oligo_class> oligos_vector;
	GEP_creation(BED_FILE, TWOBIT_FILE, GEP); 			//Function to read BED and 2Bit files and create GEP objects vector

	matrix_class JASPAR_MATRIX(JASPAR_FILE);				//Function to read JASPAR PWM file, extract value from it and create a matrix class called JASPAR_MTX
	
	vector<vector<double>> matrix;
	vector<vector<double>> matrix_log;
	vector<vector<double>> matrix_log_inverse;
	matrix = JASPAR_MATRIX.return_matrix();
	JASPAR_MATRIX.print_debug_matrix(matrix, " ");

	matrix = JASPAR_MATRIX.return_norm_matrix();
	JASPAR_MATRIX.print_debug_matrix(matrix, " NORMALIZED");

	matrix = JASPAR_MATRIX.return_inverse_norm_matrix();
	JASPAR_MATRIX.print_debug_matrix(matrix, " INVERSE NORMALIZED MATRIX");
	//Print the matrix for debugging
	matrix_log = JASPAR_MATRIX.return_log_matrix();
	JASPAR_MATRIX.print_debug_matrix(matrix_log, " LOGARITHMIC MATRIX");
	
	matrix_log_inverse = JASPAR_MATRIX.return_inverse_log_matrix();
	JASPAR_MATRIX.print_debug_matrix(matrix_log_inverse, " INVERSE LOGARITHMIC MATRIX");

	for(int i=0; i<25; i++){

	string sequence = GEP[i].return_sequence(GEP[i]);
	string sequence_inverse = GEP[i].return_sequence_inverse(GEP[i]);
	string chr_coord = GEP[i].return_chr_coord_GEP();
	int start_coord = GEP[i].return_start_coord_GEP();
//	string sequence = "AAAAAGTCTGTGGTTTAAAAAAAAAAAAAAAAAGTCTGTGGTTTAAAAAAAAAAAAAAAAAAGTCTGTGGTTTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGTCTGTGGTTTAA";
	oligo_class SHIFTING(matrix_log, sequence, chr_coord, start_coord);
	oligos_vector.emplace_back(SHIFTING);
	vector<double> oligo_scores = SHIFTING.return_oligo_scores();
	int local_position = SHIFTING.return_local_position();
	double best_score = SHIFTING.return_best_score();
	string best_oligo_seq = SHIFTING.return_best_oligo_seq();
	string chr_coord_oligo = SHIFTING.return_chr_coord_oligo();
	int start_coord_oligo = SHIFTING.return_start_coord_oligo();
	int end_coord_oligo = SHIFTING.return_end_coord_oligo();
	cout << endl;
	cout << sequence << endl;

	for(int j=0; j<oligo_scores.size(); j++){
		
		cout << oligo_scores[j] << " ";
	}
	cout << endl;
	cout << "The best oligo in sequence has a score of " << best_score << endl;
	cout << "His local position is " << local_position << endl;
	cout << "The best sequence is " << best_oligo_seq << endl;
	cout << "The global coordinates are:\n> " << chr_coord_oligo << ": " << start_coord_oligo << " - " << end_coord_oligo << endl;
	}

//	for(int i=0; i<5; i++){
//
//		cout << SHIFTING.oligo_scores[i] << endl;
//	}
//	}
//	for(int i=0; i<GEP.size();i++){

//	GEP[i].print_debug_GEP(GEP[i]);					//Print GEP vector for debugging
//	}
 return 0;
}

void bed_class::read_line(string line){				//Read line function: it takes in input each line from BED file 

	istringstream mystream(line);					//Split the line word by word and extract chromosome coordinates (chr, start, end)
	mystream >> chr_coord >> start_coord >> end_coord;		

}

void bed_class::centering_function ( int start,  int end, int parameter, const int overhead){	//Centering function: in takes start and end coordinate and re-sets them -
	//following an input parameter value (overhead added)
	int center = (start + end)/2;						
	start_coord = center - parameter;			//No overhead for start
	end_coord = center + parameter +overhead;		//Overhead for end
}


void bed_class::flag_control( int start,  int end){ 	//Flag control function: start coordinates must be < then end coordinates

	if(start > end){		//if start coordinates are > or == then end coordinates, flag is setted to 0
		flag = 0;
	}
	else{ flag = 1;}
}

void GEP_creation(string Bed_file, string Twobit_file, vector<bed_class> &GEP){		//Function to read BED and 2Bit files and create GEP object vector

	ifstream in(Bed_file); 						//Opening file in lecture mode
	TwoBit * tb;				//Creating a TwoBit* variable called tb
	tb = twobit_open(Twobit_file.c_str());					//Opening 2Bit file with twobit_open function and saved in tb 
	string line; 							//defining line string

	int n_line = 1;							//line counter initialization

	while(getline(in,line)){  					//reading input file line by line with getline function

		if(line.empty())   					//if line is empty or commented --> continue
			continue;
		if(line[0]=='#')
			continue;

		bed_class new_class(parameter,line,tb, n_line);  //Called the object constructor passing the Bed line, parameter P, twobit file tb, and the line counter n_line
		GEP.emplace_back(new_class);				//Put the new object in the GEP vector with emplace function

		n_line = n_line + 1;					//pass to next line 

	}
}

void oligo_class::shifting(vector<vector<double>> matrix, string sequence, int s_iterator){
		
	double sum_oligo = 0;
	
	if(s_iterator < sequence.size() - matrix[0].size() ) {

	for(int i=0; i< matrix[0].size(); i++){

			switch(sequence[i+s_iterator]){

				case 'A':
				       
					sum_oligo = sum_oligo + matrix[0][i];
					break;

				case 'C':
				       
					sum_oligo = sum_oligo + matrix[1][i];
					break;

				case 'G':
				       
					sum_oligo = sum_oligo + matrix[2][i];
					break;

				case 'T':
				       
					sum_oligo = sum_oligo + matrix[3][i];
					break;
				
				default:
				       
					sum_oligo = sum_oligo + o_matrix_mins[i];
					break;

		}
	}
	
	oligo_scores.emplace_back(sum_oligo);
	shifting(matrix, sequence, s_iterator+1);

	}

}

void matrix_class::read_JASPAR(string JASPAR_FILE){			//Function to read JASPAR PWM file, extract values and create a matrix class

	ifstream file(JASPAR_FILE);					//opening JASPAR PWM file
	string line;							
	while(getline(file,line)){					//For each line of the file do:

		if(line[0]=='>'){					//If line start with ">"
			istringstream mystream(line);			
			mystream >> matrix_name >> tf_name;			//Extract the first two words and put into matrix_name string variable and tf_name string variable
		}

		else{							//Else, if line does not start with ">"
			line.erase(0,line.find('[') +1);		//Take line charachters after "["...
			line.erase(line.find(']'));			//...and line charachters before "]"
			vector<double> baseQ;				//Initializing baseQ vector of double
			istringstream mystream(line);			//Splitting the line in words
			for (double num; mystream >> num;){		//Put every word(number of matrix), ricorsively, in double variable num
				baseQ.emplace_back(num);		//Put every number(num) in baseQ vector
			}
			matrix.emplace_back(baseQ);			//Put baseQ vector (corrisponding to matrix line values) in our matrix

		}

	}
	file.close();						//Closing file
}

vector<double> matrix_class::find_col_sum(vector<vector<double>> matrix){

	vector<double> col_sum;						//Vector of columns sum
	double sum = 0;							//Sum initialized as 0

	for (int i = 0; i < matrix[0].size(); i++) {			//From 0 to number of columns of line 0
		for (int j = 0; j < 4; j++){				//From 0 to 4 (line number)

			sum = sum + matrix[j][i];			//Calculate the sum of columns
		}

		col_sum.emplace_back(sum);				//Put the column sum in vector col_sum
		sum = 0;						//Restore the sum to 0 for the next column
	}
	return col_sum;
}

void matrix_class::matrix_normalization_pseudoc(vector<vector<double>> matrix, double p){  
	
	double norm;							//Norm variable initialized
	vector<double> col_sum = find_col_sum(matrix);

	for (int i = 0; i < matrix.size(); i++) {		//From 0 to number of matrix lines

		vector<double> baseQ;				//baseQ vector to store the lines initialized
		for (int j = 0; j < matrix[i].size(); j++){	//From 0 to number of matrix columns

				norm = matrix[i][j]/col_sum[j];		//Put matrix value (divided for the corresponding column sum) into double variable norm
				baseQ.emplace_back(norm + p);		//Put norm value (with p added) in baseQ vector
		}

			norm_matrix.emplace_back(baseQ);	//Put baseQ vector (which carries line values) in norm_matrix
	}
}

void matrix_class::matrix_normalization(vector<vector<double>> matrix){

	vector<double> col_sum = find_col_sum(matrix);

	for (int i = 0; i < matrix.size(); i++) {		//From 0 to number of matrix lines

		vector<double> baseQ;				//baseQ vector to store the lines initialized
		for (int j = 0; j < matrix[i].size(); j++){	//From 0 to number of matrix columns

				norm_matrix[i][j] = matrix[i][j]/col_sum[j];	//Substitution of first normalized values with new normalized ones
		}
	}
}

void matrix_class::matrix_logarithmic(vector<vector<double>> matrix){
	
	for(int i=0; i < matrix.size(); i++){
		vector<double> baseQ;
		double value_log;

		for(int j=0; j < norm_matrix[i].size(); j++){
			
			value_log = log(norm_matrix[i][j]);
			baseQ.emplace_back(value_log);
		}
		matrix_log.emplace_back(baseQ);
	}
}
	

vector<vector<double>> matrix_class::reverse_matrix(vector<vector<double>> matrix){

	vector<vector<double>> inv_matrix = matrix;
	reverse(inv_matrix.begin(), inv_matrix.end());
	for (int i = 0; i < 4; i++) {		//From 0 to number of matrix lines
		vector<double> baseQ;
		reverse(inv_matrix[i].begin(), inv_matrix[i].end());
	}
	return inv_matrix;

}

void oligo_class::find_minmax(vector<vector<double>> matrix){

	for(int i=0; i < matrix[0].size(); i++){
		vector<double> colum;		   	
		for(int j=0; j < matrix.size(); j++){
			colum.emplace_back(matrix[j][i]);
		}
		o_matrix_mins.emplace_back(*min_element(colum.begin(),colum.end()));
		o_matrix_maxes.emplace_back(*max_element(colum.begin(),colum.end()));
	}
	min_possible_score = accumulate(o_matrix_mins.begin(), o_matrix_mins.end(), 0.0);
	max_possible_score = accumulate(o_matrix_maxes.begin(), o_matrix_maxes.end(), 0.0);

}	

void oligo_class::find_best_score(vector<double> oligo_scores){
	
	best_score = *max_element(oligo_scores.begin(), oligo_scores.end());
	vector<int> positions;
	int center = parameter/2;
	int matches = 0;

	for(int i=0; i < oligo_scores.size(); i++){

		if(oligo_scores[i] == best_score){
			
			matches = matches + 1;
			positions.emplace_back(i);
		}
	}
	
	if(matches > 1){

		local_position = nearest_center(positions, center);
	}

	else{
		local_position = positions[0];
	}
}

void oligo_class::find_best_sequence(string sequence, int local_position, int length){

	char* best_seq = new char;
	strncpy(best_seq, sequence.c_str() + local_position, length);
	string best(best_seq);
	best_oligo_seq = best;
}

void oligo_class::find_coordinate(int local_position, int length, string chr_coord_GEP, int start_coord_GEP){

	chr_coord_oligo = chr_coord_GEP;
	start_coord_oligo = start_coord_GEP + local_position;
	end_coord_oligo = start_coord_oligo + length;

}


int oligo_class::nearest_center(vector<int> positions, int center){

	vector<int> save_positions = positions;
	int min;

	for(int i=0; i<positions.size(); i++){
		
		positions[i] = abs(positions[i]-center);
	}
		
	min = *min_element(positions.begin(),positions.end());
	
	for(int i=0; i<positions.size(); i++){

		if(min == positions[i]){

			return save_positions[i];
		}
	}
}

void bed_class::extract_seq(TwoBit* tb, int n_line){			//Extract sequence function: Extract, from Twobit hg38 genome, the DNA sequence with (chr, start, end) coordinates -
	//extracted from Bed line
	if(flag == 1){								//CONTROL: if flag is 1 means that the current line has starting coordinate > end coordinate, so it is correct
		string chrom = chr_coord; 				//Put in chrom the string of chr_coord
		sequence = twobit_sequence(tb,chrom.c_str(),start_coord,end_coord-1); 	//Extract the sequence from the object with the twobit_sequence function
	}
	else {		
		cerr << "WARNING: the line " << n_line <<" is omitted because starting coordinates > end coordinates, please check your BED file!" << "\n";
		//if flag is not 1 means that the current line has starting coordinate < end coordinate: PRINT WARNING!		
	}
}

void bed_class::reverse_sequence(string sequence){
	
	string sequence_inverse = sequence;

	for(int i=0; i< sequence.size(); i++){

			switch(sequence[i]){

				case 'A':

					sequence_inverse.replace(i, 1, "T");
					break;

				case 'C':
				       
					sequence_inverse.replace(i, 1, "G");
					break;

				case 'G':
				       
					sequence_inverse.replace(i, 1, "C");
					break;

				case 'T':
				       
					sequence_inverse.replace(i, 1, "A");
					break;
				
				default:
				       
					sequence_inverse.replace(i, 1, "N");
					break;
		}
	}
}

/////DEBUG/////////////////////////////////////////////////////////
		
string oligo_class::return_chr_coord_oligo(){

	return chr_coord_oligo;
}

int oligo_class::return_start_coord_oligo(){

	return start_coord_oligo;
}

int oligo_class::return_end_coord_oligo(){

	return end_coord_oligo;
}
string bed_class::return_chr_coord_GEP(){

	return chr_coord;
}

int bed_class::return_start_coord_GEP(){

	return start_coord;
}

string oligo_class::return_best_oligo_seq(){

	return best_oligo_seq;
}

int oligo_class::return_local_position(){

	return local_position;
}
vector<double> oligo_class::return_oligo_scores(){

	return oligo_scores;
}

double oligo_class::return_best_score(){

	return best_score;
}

vector<vector<double>> matrix_class::return_matrix(){

	return matrix;
}
vector<vector<double>> matrix_class::return_norm_matrix(){

	return norm_matrix;
}
vector<vector<double>> matrix_class::return_inverse_norm_matrix(){

	return inverse_norm_matrix;
}
vector<vector<double>> matrix_class::return_inverse_log_matrix(){

	return inverse_matrix_log;
}
vector<vector<double>> matrix_class::return_log_matrix(){

	return matrix_log;
}
string bed_class::return_sequence(bed_class){

       return sequence;
}
string bed_class::return_sequence_inverse(bed_class){

       return sequence_inverse;
}

void matrix_class::print_debug_matrix(vector<vector<double>> matrix, string type){			//Debugging of matrix
	
	cout << "\n" << matrix_name << " " << tf_name << type << ":" << endl;

	for(int i=0; i < matrix.size(); i++){
		for(int j=0; j<matrix[i].size(); j++){

			cout << matrix[i][j] << " ";
		}
		cout << endl;
	}
	
}


void bed_class::print_debug_GEP(bed_class){			//Debug function: Print the GEP vector to control the working flow

	cout << ">" << chr_coord << ":" << start_coord << " - " << end_coord << endl;	//Printing chr, start and end coordinates
	cout << sequence << endl;					//Printing sequence

}
///////////////////////////////////////////////////////////////////////////////////////////////


////////////////////PARSER////////////////////////////////////////////////////////////////////
void command_line_parser(int argc, char **argv){

	for(int i = 1; i < argc; i++){

		string buf = argv[i];

		if(buf == "--help" || buf == "-h"){

			display_help();

		}

		else if(buf == "--BED" || buf == "-B"){

			if(i < argc - 1){

				BED_FILE = argv[++i];
				is_file_exist(BED_FILE, buf);
				continue;

			}
		}

		else if(buf == "--param" || buf == "-p"){

			if(i < argc - 1){
				try{

				parameter = stoi(argv[++i]);}
				catch(exception &err)
				{
					cerr<< buf <<" parameter is not a number"<<endl;
				}
				continue;
			}
		}

		else if(buf == "--jaspar" || buf == "-J"){

			if(i < argc - 1){

				JASPAR_FILE = argv[++i];
				is_file_exist(JASPAR_FILE, buf);
				continue;

			}
		}

		else if(buf == "--twobit" || buf == "-tb"){

			if(i < argc - 1){

				TWOBIT_FILE = argv[++i];
				is_file_exist(TWOBIT_FILE, buf);
				continue;
			}
		}
		else
		{
			cerr << "Unknown option: " << buf <<  endl;
			exit(1);
		}

	}
}

bool is_file_exist(string fileName, string buf)		//Input files existence control
{
        struct stat check;
	int regular_check;
	const char * C_fileName = fileName.c_str();
        stat(C_fileName, &check );

	regular_check = S_ISREG( check.st_mode );

	if ( regular_check == 0 ) {
		cerr <<"ERROR: "<< buf << " parameter has wrong argument"<< endl;
		exit(1);	
	}
	return 0;
}


void display_help() 						//Display help function
{
	cerr << "\n --help show this message" << endl;
	cerr << "\n --BED -B <file_bed>: input bed file" << endl;
	cerr << "\n --twobit -tb <file_twobit>: input twobit file" << endl;
	cerr << "\n --jaspar -J <JASPAR_file>: input JASPAR file" << endl;
	cerr << "\n --param -p <parameter>: input parameter to select bases number to keep around the chip seq signal" << endl;
	cerr << endl;

	exit(EXIT_SUCCESS);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
