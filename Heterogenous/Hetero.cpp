
//Covid.cpp : Defin1es the entry point for the console application.
//
/******************************************************************************************
* This program creates formulation for the stochastic vaccine coverage problem     *
* The program reads an input data file to create LP and CORE file, TIME and STOCH files. *
*																						  *
*         Authors  : Jiangyue Gong 	                                  *
*         Date    : June 02, 2020			                                              *
*	      Revised :
*         Version:
*					                                                                      *
*					                                                                      *
******************************************************************************************/
#include "stdAfx.h" //standard header file

int main(int argc, char* argv[])
{	
	typedef struct {
		int comNames;
		char ID;
		char Area[NLEN];
		int zipCode;
		char	state[NLEN];
		int		totalHousehold; /// number of household in a community
		int* householdType;
		int population;
		//double*** ctrlFactor;
	}Community;


	typedef struct {
		int compCount;
		int size;
		int group1;
		int group2;
		int group3;
		int *count;
	}Composition;

	typedef struct {
		int size;
		float* prop;
	}Proportion;


	typedef struct {
		int compCount;
		float* prop;
		float prop1;
		float prop2;
		float prop3;
		float prop4;
		float prop5; 
		float prop6;
		float prop7;
	}NewProportion;

	typedef struct {
		float prob;
		float* sucept;
		double group1;
		double group2;
		double group3;
	}Susceptibility;


	typedef struct {
		float prob;
		float *inffect;
		double group1;
		double group2;
		double group3;
	}Ineffectivity;


	typedef struct {
		float prob;
		float contactRate;
		float transRate;
		float vacEfficacy;
	}Scen;



	typedef struct {
		float prob;
		double contactRate;
		double transRate;
		double vacEfficacy;
		float* sucept;
		float* ineffect;
	}OverScen;
	int numHouseSizeType;			// Number of household size type, here we have 7
	int numCommunities;				// Number of communities
	int* LPzip;	
	int totalComp;
	int numGroup = 3;
	int* LPcomNames;
	char buffer[BUFLEN];
	char field[FLEN];
	double totalVac;
	int lpType;
	int* numComp;
	int varcnt =0;
	double* alpha_s;
	double* alpha_e;
	double level;
	double minCoverage;	
	int lineNum=0;
	int* numScen;
	double reproductionNum;
	int numComLp;
	double *alpha;
	
	string varPrefix;			// Decision variable prefix
	string corefile_lp;		     // MIP file in Linear Programming (LP) format
	string corefile_mps;	     // MIP file in Mathematical Programming Society (MPS) format
	string solnfile;		     // MIP solution file
	string prefix;			     // Name prefix

	//char stringnum[4];

	// File streams
	ofstream fres;			// output file stream for resources availabilities
	ofstream fout;			// Output file stream for output results
	ofstream flog;			// Output file stream for log file
	string resultsfile;		// Results output file name
	string logfile;			// Log file name


	// CPLEX variables
	int status;
	CPXENVptr env = NULL;
	CPXLPptr  lp;    // Pointer to CPLEX LP obj
					 //CPXLPptr lp2;

	try {
		/////////////////////////////////////////////////////////////////////
		// Check the command line arguments and get input info
		/////////////////////////////////////////////////////////////////////

		if (argc != 4) {
			cout << " To run this program type: Community household DataFile and Lp Generation DataFile" << endl;
			cout << " at the prompt!" << endl;
			cout << " the format is described in the ReadMe.txt file that came with this code \n" << endl;
			cout << " Terminating...\n" << endl;
			return 1;
		}



		// Start wall clock
		clock_t wallclockstart = clock();


		//////////////////////////////////////////////////////////////////////
		// Initialize the CPLEX environment 
		//////////////////////////////////////////////////////////////////////
		env = CPXopenCPLEX(&status);
		// If an error occurs, the status value indicates the reason for
		// failure.  A call to CPXgeterrorstring will produce the text of
		// the error message.  Note that CPXopenCPLEX produces no
		// output, so the only way to see the cause of the error is to use
		// CPXgeterrorstring.  For other CPLEX routines, the errors will
		// be seen if the CPX_PARAM_SCRIND parameter is set to CPX_ON 
		if (env == NULL) {
			char errmsg[1024];
			CPXgeterrorstring(env, status, errmsg);
			cerr << errmsg << endl;
			exit(2);
		}
		// Turn on/off output to the screen 
		status = CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
		if (status)
		{
			cerr << "Failure to turn on screen indicator!" << endl;
			exit(2);
		}
		status = CPXsetintparam(env, CPX_PARAM_PROBE, 3);
		if (status)
		{
			cerr << "Failure to turn on screen indicator!" << endl;
			exit(2);
		}


		//CPXsetdblparam(env, CPX_PARAM_EPINT, 1e-3);


		status = CPXsetdblparam(env, CPX_PARAM_TILIM, 43200);
		if (status)
		{
			cerr << "Failure to turn on screen indicator!" << endl;
			exit(2);
		}

		//status = CPXsetdblparam(env, CPX_PARAM_EPGAP, 0.05);
		if (status)
		{
			cerr << "Failure to turn on screen indicator!" << endl;
			exit(2);
		}

		////////////////////////////////////////////////////
		//       Open and read Household data files              
		////////////////////////////////////////////////////
		// DataFile
		ifstream fin1(argv[1]);
		if (!fin1.is_open()) {
			cerr << "Unable to open data file: " << argv[1] << " for reading!" << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		else {
			cout << " Reading input file " << argv[1] << "..." << endl;
		}

		////////////////////////////////////////////////////
		//       Read PathwayDataFile file              
		////////////////////////////////////////////////////
		// Scan the comments: 3 lines
		for (int j = 0; j < 4; j++) {
			fin1.getline(buffer, BUFLEN);
			cout << buffer << endl;
			sscanf(buffer, "%s", field);
			if (strcmp(field, "#") != 0) {
				cerr << "Line " << lineNum << " of input file " << argv[1] << "must start with ""#"" " << " and NOT " << field << "." << endl;
				cerr << "Terminating ..." << endl;
				return(1);
			}
			lineNum++;
		} // end for loop

		// Read in number of communities
		fin1.getline(buffer, BUFLEN);
		sscanf(buffer, "%d", &numCommunities);
		cout << "\t numCommunities   = " << numCommunities << endl;
		lineNum++;

		// Read comment
		fin1.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

		// Read in number of household types
		fin1.getline(buffer, BUFLEN);
		sscanf(buffer, "%d", &numHouseSizeType);
		cout << "\t numHouseSizeType   = " << numHouseSizeType << endl;
		lineNum++;

		// Read comment
		fin1.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

	
		Community* mycommunity;
		mycommunity = new Community[numCommunities];

		for (int i = 0; i < numCommunities; i++)
		{
			mycommunity[i].householdType = new int[numHouseSizeType];
		}


		
		for (int i = 0; i < numCommunities; i++)
		{
			fin1.getline(buffer, BUFLEN);
			sscanf(buffer, "%d%s%s%d%s%d%d%d%d%d%d%d%d", &mycommunity[i].comNames, &mycommunity[i].ID, 
				&mycommunity[i].Area, &mycommunity[i].zipCode, &mycommunity[i].state, &mycommunity[i].totalHousehold,
				&mycommunity[i].householdType[0], &mycommunity[i].householdType[1], &mycommunity[i].householdType[2],
				&mycommunity[i].householdType[3], &mycommunity[i].householdType[4], &mycommunity[i].householdType[5], &mycommunity[i].householdType[6]);
			
			lineNum++;
		}

		// Read comment
		fin1.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;
		numComp = new int[numHouseSizeType];
		for (int i = 0; i < numHouseSizeType; i++)
		{
			fin1.getline(buffer, BUFLEN);
			sscanf(buffer, "%d", &numComp[i]);
			cout << " Number of compositions for household size  = " << numComp[i] << endl;
			lineNum++;
			
		}

		// Read comment
		fin1.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;
				
			fin1.getline(buffer, BUFLEN);
			sscanf(buffer, "%d", &totalComp);
			cout << " Number of compositions   = " << totalComp << endl;
			lineNum++;
	// Declare memory


		Composition* myComposition;
		myComposition = new Composition [totalComp];


		for (int i = 0; i < totalComp; i++)
		{
			myComposition[i].count = new int[numGroup];
		}
		// Read comment
		fin1.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

		// Read the composition
		for (int i = 0; i < totalComp; i++)
		{
			
			//cout << "num composition = " << numComp[i] << endl;
			
				fin1.getline(buffer, BUFLEN);
				sscanf(buffer, "%d%d%d%d%d", &myComposition[i].compCount, &myComposition[i].size, &myComposition[i].group1, &myComposition[i].group2, &myComposition[i].group3);
				sscanf(buffer, "%d%d%d%d%d", &myComposition[i].compCount, &myComposition[i].size, &myComposition[i].count[0], &myComposition[i].count[1], &myComposition[i].count[2]);

				lineNum++;

		}

		NewProportion* myNewProportion;

		myNewProportion = new NewProportion [totalComp];
		for (int i = 0; i < totalComp; i++)
		{
			myNewProportion[i].prop = new float[numCommunities];
		}

		// Read comment
		fin1.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

		for (int i = 0; i < totalComp; i++)
		{

			for (int j = 0; j < numHouseSizeType; j++)
			{
				if (j == 0)
				{
					fin1.getline(buffer, BUFLEN);
					sscanf(buffer, "%d%f%f%f%f%f%f%f", &myNewProportion[i].compCount, &myNewProportion[i].prop[0], &myNewProportion[i].prop[1], &myNewProportion[i].prop[2],
						&myNewProportion[i].prop[3],&myNewProportion[i].prop[4], &myNewProportion[i].prop[5], &myNewProportion[i].prop[6]);
					lineNum++;
				}

			}

		}
		
		fin1.getline(buffer, BUFLEN);
		sscanf(buffer, "%s", field);
		if (strcmp(field, "ENDATA") != 0) {
			cerr << "Expecting ""ENDATA"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		

		
		cout << " Data file " << argv[1] << " read successfully, closing file..." << endl;
		// Close input file
		fin1.close();
				/////////////////////////////////////////////
				// Read LP Generation input file
				//
				/////////////////////////////////////////
		ifstream fin2(argv[2]);
		if (!fin2.is_open()) {
			cerr << "Unable to open data file: " << argv[2] << " for reading!" << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		else {
			cout << " Reading input file " << argv[2] << "..." << endl;
		}




		// Scan the comments: 4 lines
		for (int j = 0; j < 4; j++) {
			fin2.getline(buffer, BUFLEN);
			cout << buffer << endl;
			sscanf(buffer, "%s", field);
			if (strcmp(field, "#") != 0) {
				cerr << "Line " << lineNum << " of input file " << argv[2] << "must start with ""#"" " << " and NOT " << field << "." << endl;
				cerr << "Terminating ..." << endl;
				return(1);
			}
			lineNum++;
		} // end for loop

		fin2.getline(buffer, BUFLEN);
		sscanf(buffer, "%lf", &minCoverage);
		cout << "\t minCoverage = " << minCoverage << endl;
		lineNum++;
		
		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[2] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		fin2.getline(buffer, BUFLEN);
		sscanf(buffer, "%lf", &level);
		cout << "\t level = " << level << endl;
		lineNum++;

		totalVac = minCoverage * level;

		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[2] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;
		

		// Read in number of time slots in a day
		fin2.getline(buffer, BUFLEN);
		sscanf(buffer, "%lf", &reproductionNum);
		cout << "\t reproductionNum = " << reproductionNum << endl;
		lineNum++;


		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[2] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;


		fin2.getline(buffer, BUFLEN);
		sscanf(buffer, "%d", &lpType);
		cout << "\t lpType = " << lpType << endl;
		lineNum++;


		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Line " << lineNum << " of input file " << argv[2] << "must start with ""#"" " << " and NOT " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

		// Read in number of time slots in a day
		fin2.getline(buffer, BUFLEN);
		sscanf(buffer, "%d", &numComLp);
		cout << "\tnumComLp = " << numComLp << endl;
		lineNum++;


		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[2] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;
		
		LPzip = new int [numComLp];
		
		for (int i = 0; i < numComLp; i++)
		{
			fin2.getline(buffer, BUFLEN);
			sscanf(buffer, "%d", &LPzip[i]);
			cout << "\t LPzip[" <<i<<"] = " << LPzip[i] << endl;
			lineNum++;
		}

		alpha_e = new double[numCommunities];
		
		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[2] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

		for (int i = 0; i < numCommunities; i++)
		{
			fin2.getline(buffer, BUFLEN);
			sscanf(buffer, "%lf", &alpha_e[i]);
			cout << "\t alpha_e[" << i << "] = " << alpha_e[i] << endl;
			lineNum++;
		}



		alpha_s = new double[numCommunities];
		fin2.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[2] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;

		for (int i = 0; i < numCommunities; i++)
		{
			fin2.getline(buffer, BUFLEN);
			sscanf(buffer, "%lf", &alpha_s[i]);
			cout << "\t alpha_s[" << i << "] = " << alpha_s[i] << endl;
			lineNum++;
		}

		

		fin2.getline(buffer, BUFLEN);
		sscanf(buffer, "%s", field);
		if (strcmp(field, "ENDATA") != 0) {
			cerr << "Expecting ""ENDATA"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}


		cout << " Data file " << argv[2] << " read successfully, closing file..." << endl;
		// Close input file
		fin2.close();



		



		ifstream fin3(argv[3]);
		if (!fin3.is_open()) {
			cerr << "Unable to open data file: " << argv[3] << " for reading!" << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		else {
			cout << " Reading input file " << argv[3] << "..." << endl;
		}



		// Scan the comments: 4 lines
		for (int j = 0; j < 4; j++) {
			fin3.getline(buffer, BUFLEN);
			cout << buffer << endl;
			sscanf(buffer, "%s", field);
			if (strcmp(field, "#") != 0) {
				cerr << "Line " << lineNum << " of input file " << argv[3] << "must start with ""#"" " << " and NOT " << field << "." << endl;
				cerr << "Terminating ..." << endl;
				return(1);
			}
			lineNum++;
		} // end for loop

		numScen = new int[numCommunities];

		for (int c = 0; c < numCommunities; c++)
		{
			fin3.getline(buffer, BUFLEN);
			sscanf(buffer, "%d", &numScen[c]);
			cout << "\t numScen   = " << numScen[c] << endl;
			lineNum++;
		}


		Scen** myscen;
		myscen = new Scen * [numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			myscen[i] = new Scen[numScen[i]];
		}


		

	
		Susceptibility** mySuscept;
		mySuscept = new Susceptibility * [numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			mySuscept[i] = new Susceptibility[numScen[i]];
		}
		for (int i = 0; i < numCommunities; i++)
		{
			for (int j = 0; j < numScen[i]; j++)
			{
				mySuscept[i][j].sucept = new float[numGroup];
			}
		}


		Ineffectivity** myIneffect;
		myIneffect = new Ineffectivity * [numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			myIneffect[i] = new Ineffectivity[numScen[i]];
		}


		for (int i = 0; i < numCommunities; i++)
		{
			for (int j = 0; j < numScen[i]; j++)
			{
				myIneffect[i][j].inffect = new float[numGroup];
			}

		}



		for (int c = 0; c < numCommunities; c++)
		{


			// Read comment
			fin3.getline(buffer, BUFLEN);
			cout << buffer << endl;
			sscanf(buffer, "%s", field);
			if (strcmp(field, "#") != 0) {
				cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[3] << " but found " << field << "." << endl;
				cerr << "Terminating ..." << endl;
				return(1);
			}
			lineNum++;

			for (int i = 0; i < numScen[c]; i++)
			{
				fin3.getline(buffer, BUFLEN);
				sscanf(buffer, "%f%f%f%f%f%f%f%f%f%f", &myscen[c][i].prob, &myscen[c][i].contactRate, &myscen[c][i].transRate, &myscen[c][i].vacEfficacy,
				 &mySuscept[c][i].sucept[0], &mySuscept[c][i].sucept[1], &mySuscept[c][i].sucept[2],
					&myIneffect[c][i].inffect[0], &myIneffect[c][i].inffect[1], &myIneffect[c][i].inffect[2]);
				//cout << " For s = " << i << "\t prob = " << myscen[j][i].prob << "\t contactRate = " << myscen[j][i].contactRate << "\t transRate =" << myscen[j][i].transRate << " \t efficacy = " << myscen[j][i].vacEfficacy << endl;

				lineNum++;
			}
		}








		alpha = new double[numCommunities];

		// Read comment
		fin3.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[3] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;


		for (int i = 0; i < numCommunities; i++)
		{
			fin3.getline(buffer, BUFLEN);
			sscanf(buffer, "%lf", &alpha[i]);
			cout << "alpha [" << i << "]  = " << alpha[i] <<  endl;

			lineNum++;
		}

		
		fin3.getline(buffer, BUFLEN);
		sscanf(buffer, "%s", field);
		if (strcmp(field, "ENDATA") != 0) {
			cerr << "Expecting ""ENDATA"" on line " << lineNum << " of input file " << argv[3] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}


		cout << " Data file " << argv[3] << " read successfully, closing file..." << endl;
		// Close input file
		fin3.close();

		///// Calculate the mean household size in a community

		float* avgSize;
		avgSize = new float[numCommunities];

		float** prop;
		prop = new float*[numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			prop[i] = new float[numHouseSizeType];
		}



		for (int i = 0; i < numCommunities; i++)
		{


			for (int j = 0; j < numHouseSizeType; j++)
			{
				prop[i][j] = 0;
			}
		}

		
		
			for (int j = 0; j < totalComp; j++)
			{
				for (int i = 0; i < numCommunities; i++)
				{
					avgSize[i] = avgSize[i] + myNewProportion[j].prop[i] * myComposition[j].size;
					
				}
				
								

			}
		
		
		for (int i = 0; i < numCommunities; i++)
		{
			cout << "avgSize[" << i << "] = " << avgSize[i] << endl;
		}

		

		

		////////////////////////////////////////////////////////////////////
		// Write MIP formulation to file
		//
		// NOTE: The MIP is created as an LP file and is then read in 
		// and converted to MPS format for the CORE file
		///////////////////////////////////////////////////////////////////




		prefix = "14C\\DEP";
		corefile_lp = prefix;
		corefile_lp += ".lp";

		ofstream fcore_d(corefile_lp.c_str());
		if (!fcore_d.is_open()) {
			cerr << "Unable to open file: " << corefile_lp << " for writing!" << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}

		solnfile = prefix;
		solnfile += ".sol";
		ofstream fsoln_d(solnfile.c_str());
		if (!fsoln_d.is_open()) {
			cerr << "Unable to open file: " << solnfile << " for writing!" << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}

		
		
		fcore_d << "Min \n obj:  ";
		varcnt = 0;
		int bigM =  5000;
		int bigM2 = 5000000;
		int index;



		LPcomNames = new int[numComLp];

		for (int i = 0; i < numComLp; i++)
		{
			LPcomNames[i] = 0;
		}

		for (int i = 0; i < numCommunities; i++)
		{
			for (int j = 0; j < numComLp; j++)
			{
				if (mycommunity[i].zipCode == LPzip[j])
				{
					LPcomNames[j] = mycommunity[i].comNames;
					cout << "Community name = " << mycommunity[i].comNames << endl;
					varcnt++;
					if (varcnt % 11 == 0)
					{
						fcore_d << endl;
					}
				}
			}
		}



		varcnt = 0;
	

	
		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c]; /// County index
			for (int n = 0; n < totalComp; n++)
			{
					for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
					{
						for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
						{

							for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
							{
								//if (v1 + v2 + v3 > 0 and myNewProportion[n].prop[index - 1] > 0)
								if (myNewProportion[n].prop[index - 1] > 0)
								{


									fcore_d << " + " << (v1 + v2 + v3) * myNewProportion[n].prop[index - 1] << " x_" << n + 1 << "_"
										<< v1 << "_" << v2 << "_" << v3 << "_" << index;
									//fcore_d << " + " <<v1+v2+ v3 << " * " << myNewProportion[n].prop[index-1] << " x_" << n + 1 << "_"
									//		<< v1<< "_" << v2 << "_" << v3 << "_" << index;
									varcnt++;
									if (varcnt % 5 == 0)
									{
										fcore_d << endl;
									}
								}
							}
						}
						
					}
			}
			fcore_d << endl;

		}
		

		if (lpType ==1)
		{
			for (int c = 0; c < numComLp; c++)
			{
				index = LPcomNames[c];
				fcore_d << "- " << bigM << " alpha_e" << index << endl;
			}

			for (int c = 0; c < numComLp; c++)
			{
				index = LPcomNames[c];
				fcore_d << "+ " << bigM2 << " alpha_s" << index << endl;
			}
		}
		


		

		// *** Write the constraints *** //
		fcore_d << "Subject To" << endl;
		int cnt_d = 1;
		int xcnt_d = 1; // counts the number of x decision variables
		//int totalCompType = myComposition[6][35].compCount;
		//cout << "totalCompType = " << totalCompType << endl;
		cerr << " Writing first constraint..." << endl;
		varcnt = 0;
		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];

			for (int n = 0; n < totalComp; n++)
			{

				if (myNewProportion[n].prop[index - 1] > 0)
				{



					fcore_d << "cx_" << cnt_d << " : ";
					for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
					{
						for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
						{

							for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
							{
								fcore_d << " +  x_" << n + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
								varcnt++;
								if (varcnt % 10 == 0)
								{
									fcore_d << endl;
								}
							}
						}

					}


					fcore_d << " = 1" << endl;
					cnt_d++;

				}
			}
		}
	
	
		cerr << " Writing Second constraint..." << endl;

		varcnt = 0;
		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];

			for (int s = 0; s < numScen[index - 1]; s++)
			{

				fcore_d << "c2_" << cnt_d << " : ";
				for (int i = 0; i < totalComp; i++)
				{
					
					
						for (int v1 = 0; v1 < myComposition[i].group1 + 1; v1++)
						{
							for (int v2 = 0; v2 < myComposition[i].group2 + 1; v2++)
							{
								for (int v3 = 0; v3 < myComposition[i].group3 + 1; v3++)
								{
									float term1, term2, term3;

									term1 = mySuscept[index - 1][s].sucept[0] * myIneffect[index - 1][s].inffect[0] * (1 -  myscen[index - 1][s].transRate )
									* ( myComposition[i].group1  -  v1 *  myscen[index - 1][s].vacEfficacy ) 
										+ mySuscept[index - 1][s].sucept[1] * myIneffect[index - 1][s].inffect[1]* (1 -  myscen[index - 1][s].transRate )
									 * ( myComposition[i].group2   -  v2  *  myscen[index - 1][s].vacEfficacy  ) 
									+ mySuscept[index - 1][s].sucept[2]   *  myIneffect[index - 1][s].inffect[2]  * (1 -   myscen[index - 1][s].transRate  )
									 * (  myComposition[i].group3   -  v3  *  myscen[index - 1][s].vacEfficacy  ) ;

									////////////Term 2
	
									term2 = + mySuscept[index - 1][s].sucept[0]   *  myIneffect[index - 1][s].inffect[0]  *   myscen[index - 1][s].transRate
									  *   v1  *  myscen[index - 1][s].vacEfficacy   * ( 1 -   myscen[index - 1][s].vacEfficacy  )  
									  + mySuscept[index - 1][s].sucept[1]   *  myIneffect[index - 1][s].inffect[1]  *   myscen[index - 1][s].transRate
									  *   v2  *  myscen[index - 1][s].vacEfficacy   * ( 1 -   myscen[index - 1][s].vacEfficacy  )  
									  + mySuscept[index - 1][s].sucept[2]   *  myIneffect[index - 1][s].inffect[2]  *   myscen[index - 1][s].transRate
									  *v3* myscen[index - 1][s].vacEfficacy* (1 - myscen[index - 1][s].vacEfficacy);

									//////////////////TERM 3

									term3 = myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[0] * myIneffect[index - 1][s].inffect[0]
										* (myComposition[i].group1 - v1 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group1 - v1 * myscen[index - 1][s].vacEfficacy)


										+ myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[0] * myIneffect[index - 1][s].inffect[1]
										* (myComposition[i].group1 - v1 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group2 - v2 * myscen[index - 1][s].vacEfficacy)


										+ myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[0] * myIneffect[index - 1][s].inffect[2]
										* (myComposition[i].group1 - v1 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group3 - v3 * myscen[index - 1][s].vacEfficacy)


										+myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[1] * myIneffect[index - 1][s].inffect[0]
										* (myComposition[i].group2 - v2 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group1 - v1 * myscen[index - 1][s].vacEfficacy)


										+ myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[1] * myIneffect[index - 1][s].inffect[1]
										* (myComposition[i].group2 - v2 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group2 - v2 * myscen[index - 1][s].vacEfficacy)


										+ myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[1] * myIneffect[index - 1][s].inffect[2]
										* (myComposition[i].group2 - v2 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group3 - v3 * myscen[index - 1][s].vacEfficacy)

										+myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[2] * myIneffect[index - 1][s].inffect[0]
										* (myComposition[i].group3 - v3 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group1 - v1 * myscen[index - 1][s].vacEfficacy)

										+ myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[2] * myIneffect[index - 1][s].inffect[1]
										* (myComposition[i].group3 - v3 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group2 - v2 * myscen[index - 1][s].vacEfficacy)


										+ myscen[index - 1][s].transRate * mySuscept[index - 1][s].sucept[2] * myIneffect[index - 1][s].inffect[2]
										* (myComposition[i].group3 - v3 * myscen[index - 1][s].vacEfficacy)
										* (myComposition[i].group3 - v3 * myscen[index - 1][s].vacEfficacy);

									
									if (myNewProportion[i].prop[index - 1] > 0 and term1 + term2 + term3 >0)
									{

										if (myscen[index - 1][s].contactRate * myNewProportion[i].prop[index - 1] / avgSize[index - 1] * (term1 + term2 + term3)>0.0000000001)
										{
											fcore_d << " + " << myscen[index - 1][s].contactRate * myNewProportion[i].prop[index - 1] / avgSize[index - 1] * (term1 + term2 + term3) << "x_" << i + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
											varcnt++;
										}
										varcnt++;
										if (varcnt % 11 == 0)
										{
											fcore_d << endl;
										}
									}
									
									
								}
							}


						}


				}

				fcore_d << " - " << bigM << "z" << s + 1<<"_"<<index << "  <= 1" << endl;
				cnt_d++;

			}

		}

		cerr << " Writing third constraint..." << endl;
		varcnt = 0;
		for (int i = 0; i < numComLp; i++)
		{
			index = LPcomNames[i];

			fcore_d << " cz" << cnt_d << " : ";

			for (int s = 0; s < numScen[index-1]; s++)
			{
				
					fcore_d << " + " << myscen[index-1][s].prob << " z" << s + 1 << "_" <<index;
					varcnt++;
					if (varcnt % 11 == 0)
					{
						fcore_d << endl;
					}

			}
		
		

			if (lpType == 0)
			{
				fcore_d <<" <= " <<1- alpha[index-1] << endl;
				cout << "alpha [" << index << "] = "<< alpha[index - 1] << endl;
			}
			if (lpType ==1)
			{
				fcore_d << " +  alpha_e" << index << " -  alpha_s" << index << " = " << 1- alpha[index-1] << endl;
			}
			
			cnt_d++;
		}


		varcnt = 0;
		cerr << " Writing fourth constraint..." << endl;
		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];

				fcore_d << "cT_" << cnt_d << " : ";
				for (int n = 0; n < totalComp; n++)
				{
					//int size = myComposition[n].size;
					

					
					for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
					{
						for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
						{

							for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
							{
								if (myNewProportion[n].prop[index - 1] > 0 and v1 + v2 + v3 > 0)
								{
									fcore_d << "+" << mycommunity[index - 1].totalHousehold * myNewProportion[n].prop[index - 1] * (v1 + v2 + v3) << "   x_" << n + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
									varcnt++;
									if (varcnt % 11 == 0)
									{
										fcore_d << endl;
									}
								}
							}
						}

					}


				}

			fcore_d << " -" << "T" << index << " =  0" << endl;
			cnt_d++;
		}
		if (lpType == 1)
		{
			cerr << " Writing 5th constraint..." << endl;
			fcore_d << " cT_" << cnt_d << " : ";
			for (int c = 0; c < numComLp; c++)
			{
				index = LPcomNames[c];

				fcore_d << " + T" << index;
			}
			fcore_d << " <=  " << totalVac << endl;
			cnt_d++;
		}


		varcnt = 0;
		cerr << " Writing cp constraint..." << endl;
		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];

			for (int n = 0; n < totalComp; n++)
			{
				fcore_d << "cp_" << cnt_d << " : ";
				//int size = myComposition[n].size;

				for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
				{
					for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
					{

						for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
						{
							if (myNewProportion[n].prop[index - 1] > 0 and v1 > 0)
							{
								fcore_d << "+" << mycommunity[index - 1].totalHousehold * myNewProportion[n].prop[index - 1] * (v1) << "   x_" << n + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
								varcnt++;
								if (varcnt % 11 == 0)
								{
									fcore_d << endl;
								}
							}
						}
					}

				}

				fcore_d << " -" << "Pa"<<n+1<<"_" << index << " =  0" << endl;
				cnt_d++;
			}

			for (int n = 0; n < totalComp; n++)
			{
				fcore_d << "cp_" << cnt_d << " : ";
				//int size = myComposition[n].size;

				for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
				{
					for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
					{

						for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
						{
							if (myNewProportion[n].prop[index - 1] > 0 and v2  > 0)
							{
								fcore_d << "+" << mycommunity[index - 1].totalHousehold * myNewProportion[n].prop[index - 1] * (v2) << "   x_" << n + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
								varcnt++;
								if (varcnt % 11 == 0)
								{
									fcore_d << endl;
								}
							}
						}
					}

				}

				fcore_d << " -" << "Pb" << n+1 << "_" << index << " =  0" << endl;
				cnt_d++;
			}



			for (int n = 0; n < totalComp; n++)
			{
				fcore_d << "cp_" << cnt_d << " : ";
				//int size = myComposition[n].size;

				for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
				{
					for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
					{

						for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
						{
							if (myNewProportion[n].prop[index - 1] > 0 and v3 > 0)
							{
								fcore_d << "+" << mycommunity[index - 1].totalHousehold * myNewProportion[n].prop[index - 1] * (v3) << "   x_" << n + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
								varcnt++;
								if (varcnt % 11 == 0)
								{
									fcore_d << endl;
								}
							}
						}
					}

				}

				fcore_d << " -" << "Pc" << n+1 << "_" << index << " =  0" << endl;
				cnt_d++;
			}
			
		}

		
		

		varcnt = 0;

		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];
			fcore_d << " c" << cnt_d++ << ": - y_" << index;
			for (int n = 0; n < totalComp; n++)
			{
				for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
				{
					for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
					{

						for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
						{
							if (myNewProportion[n].prop[index - 1] > 0 and v1 + v2 + v3 > 0)
							{
								fcore_d << " + " << (v1 + v2 + v3) * myNewProportion[n].prop[index - 1] << " x_" << n + 1 << "_" << v1 << "_" << v2 << "_" << v3 << "_" << index;
								varcnt++;
								if (varcnt % 11 == 0)
								{
									fcore_d << endl;
								}
							}
						}
					}

				}
				//cout << "hn for community " << index << " household size " << n + 1 << " = " << mycommunity[index - 1].typePercent[n] << endl;

			}
			fcore_d << " = 0 " << endl;

		}
	


		int cnt;
		for (int c = 0; c < numComLp; c++)
		{
			cnt = 0;
			index = LPcomNames[c];
			for (int j = 0; j < numHouseSizeType; j++)
			{
				fcore_d << " cC_" << cnt_d << " : ";
				fcore_d << "Ga_" << j + 1 << "_" << index;
				for (int k = 0; k < numComp[j]; k++)
				{
					cnt++; /// composition type

					fcore_d << "- Pa" << cnt << "_" << index;
				}
				fcore_d << " = 0" << endl;
				cnt_d++;

			}



		}
		for (int c = 0; c < numComLp; c++)
		{
			cnt = 0;
			index = LPcomNames[c];
			for (int j = 0; j < numHouseSizeType; j++)
			{
				fcore_d << " cC_" << cnt_d << " : ";
				fcore_d << "Gb_" << j + 1 << "_" << index;
				for (int k = 0; k < numComp[j]; k++)
				{
					cnt++; /// composition type

					fcore_d << "- Pb" << cnt << "_" << index;
				}
				fcore_d << " = 0" << endl;
				cnt_d++;

			}

		}



		for (int c = 0; c < numComLp; c++)
		{
			cnt = 0;
			index = LPcomNames[c];
			for (int j = 0; j < numHouseSizeType; j++)
			{
				fcore_d << " cC_" << cnt_d << " : ";
				fcore_d << "Gc_" << j + 1 << "_" << index;
				for (int k =0; k <numComp[j]; k++)
				{
					cnt++; /// composition type
					
					fcore_d << "- Pc" << cnt  << "_" << index;
				}
				fcore_d << " = 0" << endl;
				cnt_d++;
				
			}	
		}


		if (lpType == 1)
		{
			fcore_d << "Bounds" << endl;
			for (int c = 0; c < numComLp; c++)
			{
				index = LPcomNames[c];
				fcore_d << "alpha_e" << index << " <= " << 1 - alpha[index-1] << endl;
				fcore_d << "alpha_e" << index << " <= "<<alpha_e[index-1] << endl;
				fcore_d << "alpha_s" << index << " <= "<< alpha_s[index - 1] << endl;
			}

		}
			
	
		fcore_d << "Binaries" << endl;

		for (int i = 0; i < numComLp; i++)
		{

			index = LPcomNames[i];

			for (int s = 0; s < numScen[index-1]; s++)
			{
				fcore_d << " z" << s + 1 << "_" << index << endl;
			}

		}


		fcore_d << "END" << endl;
		///////////////////////////////////////////////////////////////
		// Close the file
		fcore_d.close();
		lp = CPXcreateprob(env, &status, "core");
		if (lp == NULL) {
			cerr << endl << "Failed to create core LP." << endl;
			cerr << "CPLEX error code: " << status << endl;
			return status;
		}


		// Now read the file, and copy the data into the created lp
		status = CPXreadcopyprob(env, lp, corefile_lp.c_str(), NULL);
		if (status) {
			cerr << endl << "Failed to read and copy problem data from file " << corefile_lp << endl;
			cerr << "CPLEX status: " << status << endl;
			return status;
		}

		// Now write LP file in MPS format
		corefile_mps = prefix;
		corefile_mps += ".mps";
		cout << "\n Writing CORE file to: " << corefile_lp << endl;
		

		if (status) {
			cerr << endl << " Failed to write CORE LP in MPS format to file " << corefile_mps << endl;
			return status;
		}


		cout << endl << " File writing successful!" << endl << endl;




		///////////////////////////////////////////////////////////////////////////
		// Solve MIP
		///////////////////////////////////////////////////////////////////////////
		cout << " Now solving MIP... " << endl;
		clock_t cpustart = clock();
		status = CPXmipopt(env, lp);
		clock_t cpuend = clock();
		if (status) {
			cerr << "Failed to optimize the lp problem!" << endl;
			cerr << " CPLEX error code " << status << endl;
			return (status);
		}


		//mip_cputime = (double)(cpuend - cpustart) / CLOCKS_PER_SEC;
		//cputime += mip_cputime;

		cerr << "Done optimizing MIP " << corefile_lp << "." << endl << endl;
		//cerr << " MIP CPU time: " << mip_cputime << " secs." << endl;
		//cerr << " Total MIP CPU time: " << cputime << " secs." << endl;
		int solstat = CPXgetstat(env, lp);
		cerr << " Solution status " << status << endl;

		if (solstat == CPXMIP_INFEASIBLE) { // 
			cerr << "Failed to solve MIP: infeasible!!" << endl;
			//cerr << " Cannot schedule clientID: " << clientid + 1 << endl;
		}
		if (solstat == CPXMIP_OPTIMAL_TOL) {
			cerr << "Solved MIP to optimality within epgap  or epagap" << endl;
			cerr << " CPLEX solution status code " << solstat << endl;
		}

		// Allocate memory
		int cur_ncols = CPXgetnumcols(env, lp);
		int cur_nrows = CPXgetnumrows(env, lp);
		int surplus;
		double* mysoln = new double[cur_ncols];
		char** colnames = new char* [cur_ncols];
		for (int j = 0; j < cur_ncols; j++)
			colnames[j] = new char[NLEN];
		int colnamespace = cur_ncols * NLEN;
		char* colnamestore = new char[colnamespace];

		cout << " Number of columns: " << cur_ncols << endl;
		cout << " Number of rows:    " << cur_nrows << endl;
		

		// Get solution
		status = CPXgetmipx(env, lp, mysoln, 0, cur_ncols - 1);
		if (status) {
			cerr << " Failed to obtain solution. CPLEX error: " << status << endl;
			cerr << " Bailing out ..." << endl;
			return status;
		}

		// Get colnames
		status = CPXgetcolname(env, lp, colnames, colnamestore, colnamespace, &surplus, 0,
			cur_ncols - 1);
		if (status) {
			cerr << "Failure to CPLEX call CPXgetcolname(.), error code: " << status << endl;
			cerr << "Additional space needed for colnames array: surplus = " << surplus << endl;
			return(status);
		}



		for (int j = 0; j < cur_ncols; j++) {

			fsoln_d << " " << colnames[j] << " = " << mysoln[j] << endl;
		}


		fsoln_d.close();

		
		string resultfile;
		resultfile = "14C\\results_";
		resultfile += to_string(lpType);
		resultfile += "_";
		resultfile += to_string(level);
		resultfile += "_";
		resultfile += to_string(alpha[0]);
		resultfile += ".csv";

		ofstream myFile(resultfile.c_str());
		cout << "Now writing the solutino file" << endl;
		myFile << "lptype = " << lpType << endl;
		myFile << " total vac =" << totalVac << endl;
		for (int i = 0; i < numCommunities; i++)
		{
			myFile << "alpha level = "<<"," << alpha[i] << "," << "alpha_s bounds = "<<"," << alpha_s[i] << "," << "alpha_e bounds ="<<"," << alpha_e[i] << endl;

		}

		//ofstream myFile("solution.csv");
		int count = 0;



		
		for (int i = 0; i < cur_ncols; i++)
		{
			if (colnames[i][0] == 'a')
				
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}

		for (int i = 0; i < cur_ncols; i++)
		{
			if ( colnames[i][0] == 'T')
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}

		for (int i = 0; i < cur_ncols; i++)
		{
			if ( colnames[i][0] == 'y')
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}
		





		for (int i = 0; i < cur_ncols; i++)
		{
			if ( colnames[i][0] == 'G')
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}


		for (int i = 0; i < cur_ncols; i++)
		{
			if (colnames[i][0] == 'z')
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}



		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];

			for (int n = 0; n < totalComp; n++)
			{
				for (int v1 = 0; v1 < myComposition[n].group1 + 1; v1++)
				{
					for (int v2 = 0; v2 < myComposition[n].group2 + 1; v2++)
					{

						for (int v3 = 0; v3 < myComposition[n].group3 + 1; v3++)
						{
							myFile << colnames[count] << "," << mysoln[count] << ",";
							count++;
						}
					}

				}
				myFile << endl;
			}


		}

		myFile.close();		


	}
	catch (...)
	{
		cerr << "Exception thrown....something went wrong in the program ..." << endl;

	} // end catch
	return 0;
}

