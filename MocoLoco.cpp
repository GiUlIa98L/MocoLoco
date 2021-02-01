#include "MocoLoco.h"

int main(int argc, char *argv[]){


	if(argc == 1){             //If arguments number is 1 means that no input file has been inserted - display help
		display_help();
	}

	command_line_parser(argc, argv); //parser function called to handle aguments

	GEP_objects_creation(BED_FILE, TWOBIT_FILE); //function to read BED e 2Bit files and create GEP objects vector


}

void GEP_objects_creation(const char* Bed_file, const char* Twobit_file){
	
	ifstream in(Bed_file); //Opening file in lecture mode
	const char * chrom;   //Initializing a const char pointer called chrom 
	TwoBit * tb;		//Creating a TwoBit* variable called tb
	tb = twobit_open(Twobit_file); //Opening 2Bit file with twobit_open function and saved in tb 

	vector<genomic_position> GEP;	 //defining vector of genomic_position datas
	string line; 			//defining line string
	int n_line = 0;			//line counter initialization
	
	while(getline(in,line)){  //reading input file line by line with getline function
		
		//mettere controllo che linea non sia vuota o commentata
	
		genomic_position new_class(parameter,line);  //Called the object constructor passing the Bed line and p
		
		if(new_class.flag == 1){	//CONTROL: if flag is 1 means that the current line has starting coordinate > end coordinate, so it is correct
		        chrom = &*new_class.chr_coord.begin(); //Put in chrom the string of chr_coord
			new_class.sequence = twobit_sequence(tb,chrom,new_class.start_coord,new_class.end_coord-1); //Extract the sequence from the object with the twobit_sequence function
			GEP.push_back(genomic_position{new_class});	//put the current object in GAP (vector of objects of genomic_position class)
		}

		else {		
			cerr << "WARNING: the line " << n_line << " is omitted because starting coordinates > end coordinates, please check your BED file!" << "\n";
			//if flag is not 1 means that the current line has starting coordinate < end coordinate: PRINT WARNING!		
		}

		n_line = n_line + 1;			//pass to next line 
	}



	for (int i=0; i<GEP.size(); ++i){    // from 0 to GEP vector length
                 cout << ">" << GEP[i].chr_coord <<":"<< GEP[i].start_coord << "-" << GEP[i].end_coord << "\n";
                 //cout<< GEP[i].chr_coord <<"\t"<< GEP[i].start_coord << "\t" << GEP[i].end_coord << "\n";
		 cout << GEP[i].sequence<<"\n";
	
	 	 //cout<< GEP[i].sequence; //control print
	}

}

void command_line_parser(int argc, char **argv){

	int control_bed = 0;
	int control_twobit = 0;
	int control_p = 0;

	for(int i = 1; i < argc; i++){

		string buf = argv[i];

		if(buf == "--help" || buf == "-h"){

			display_help();

		}

		if(buf == "--BED" || buf == "-B"){

			if(i < argc - 1){

				BED_FILE = argv[++i];
				control_bed = 1;
				
				bool bed_check = is_file_exist(BED_FILE);
				if(bed_check == 0){
					cout << "File BED does not exist, please insert a BED file as input. \n";
					cout << "FATAL ERROR \n";
					exit(EXIT_SUCCESS);
				}
				continue;
			
			}
		}

		if(buf == "--param" || buf == "-p"){

			if(i < argc - 1){

				parameter = stoi(argv[++i]);
				control_p = 1;
				continue;
			}
		}
		if(buf == "--twobit" || buf == "-tb"){

			if(i < argc - 1){

				TWOBIT_FILE = argv[++i];
				control_twobit = 1;

				bool two_bit_check = is_file_exist(TWOBIT_FILE);
				if(two_bit_check == 0){
					cout << "File 2bit does not exist, please insert a 2bit file as input. \n";
					cout << "FATAL ERROR \n";
					exit(EXIT_SUCCESS);
				}
				continue;
			}
		}


	}

	if(control_bed == 0 && control_twobit == 0){
		
		cout << "BED file and TwoBit file missed or wrong parameters calling!\n";
		cout << "Please insert as input a BED file using -B or --BED annotation before it.\n";
		cout << "Please insert as input a Twobit file using -tb or --twobit annotation before it.\n";
		
		exit(EXIT_SUCCESS);
	}
		
	if(control_bed == 0){
		
		cout << "Wrong BED file immission or BED file missed!\n ";
		cout << "Please insert as input a BED file using -B or --BED annotation before it.\n";
		
		exit(EXIT_SUCCESS);
	}

	if(control_twobit == 0){

		cout << "Wrong Twobit file immission or Twobit file missed!\n";
		cout << "Please insert as input a Twobit file using -tb or --twobit annotation before it.\n";

		exit(EXIT_SUCCESS);

	}

	if(control_p == 0){

		cout << "WARNING: Sequence length parameter not inserted --> Default parameter is 150.\n";
		cout << "To put a parameter as input write -p or --param before parameter value.";
	
	}
}

bool is_file_exist(const char *fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

void display_help()
{
	cerr << "\n --help: show this message" << endl;
	cerr << "\n --BED -B <file_bed>: input bed file" << endl;
	cerr << endl;

	exit(EXIT_SUCCESS);
}

