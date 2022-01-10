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
#include "RandomGenerator.h" 
#include "NormalDistribution.h"
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
		double* typePercent;
		double** ctrlFactor;
		int population;
	}Community;


	typedef struct {
		double prob;
		double contactRate;
		double transRate;
		double vacEfficacy;
		
	}Scen;

	//int bigM = 5000;
	int* LPcomNames;
	int joint;
	int modelType;
	double totalVac;
	int* LPzip;
	int varcnt =0;
	//Input file variables
	char buffer[BUFLEN];
	char field[FLEN];
	//int temp1;
	double meanHouseSize = 0;
	//Pathway data variables
	int numHouseType;			// Number of Product types = N
	int numCommunities;				// Demand for each product [N]
	int lineNum=0;
	int temp1;
	//int numScen;
	int* numScen;
	//double contactRate;
	//double transRate;
	//double vacEfficacy;
	double reproductionNum;
	int numComLp;
	double *alpha;

	double* alpha_s;
	double* alpha_e;
	double level;
	double minCoverage;
	int lpType;

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



		// Turn on/off output to the screen 
		status = CPXsetdblparam(env, CPX_PARAM_TILIM, 120);
		if (status)
		{
			cerr << "Failure to turn on screen indicator!" << endl;
			exit(2);
		}
		







		////////////////////////////////////////////////////
		//       Open and read input data files              
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


		  

		// Read in number of household types
		fin1.getline(buffer, BUFLEN);
		sscanf(buffer, "%d", &numHouseType);
		cout << "\t numHouseType   = " << numHouseType << endl;
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

		Community* mycommunity;
		mycommunity = new Community[numCommunities];

		for (int i = 0; i < numCommunities; i++)
		{
			mycommunity[i].householdType = new int[numHouseType];
			mycommunity[i].typePercent = new double[numHouseType];
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

		LPzip = new int[numComLp];

		for (int i = 0; i < numComLp; i++)
		{
			fin2.getline(buffer, BUFLEN);
			sscanf(buffer, "%d", &LPzip[i]);
			cout << "\t LPzip[" << i << "] = " << LPzip[i] << endl;
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


		double* avgSize;
		avgSize = new double[numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			avgSize[i] = 0;

			for (int j = 0; j < numHouseType; j++)
			{
				
				mycommunity[i].typePercent[j] = double(mycommunity[i].householdType[j]) / double((mycommunity[i].totalHousehold));
				meanHouseSize =  mycommunity[i].typePercent[j] * ((double)j + 1);
				mycommunity[i].population += mycommunity[i].householdType[j] * (j + 1);
				avgSize[i] = avgSize[i] + meanHouseSize;
				}
			cout << " mean household size for community "<<mycommunity[i].comNames <<" is ="  << avgSize[i] << endl;
			cout << "Population for community [" << i + 1 << "] = " << mycommunity[i].population << endl;
			
		}

		

		
		
		for (int i = 0; i < numCommunities; i++)
		{
			mycommunity[i].ctrlFactor = new double* [numHouseType];
		}
		
		for (int i = 0; i < numCommunities; i++)
		{
			for (int j = 0; j < numHouseType; j++)
			{
				mycommunity[i].ctrlFactor[j] = new double[numHouseType + 1];
		}
			
			
		}

		



		LPcomNames = new int[numComLp];

		for (int i = 0; i < numComLp; i++)
		{
			LPcomNames[i] = 0;
		}


		////////////////////////////////////////////////////////////////////
		// Write MIP formulation to file
		//
		// NOTE: The MIP is created as an LP file and is then read in 
		// and converted to MPS format for the CORE file
		///////////////////////////////////////////////////////////////////
		
		

		int index;







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
				cerr << "Line " << lineNum << " of input file " << argv[1] << "must start with ""#"" " << " and NOT " << field << "." << endl;
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
		myscen = new Scen*[numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			myscen[i] = new Scen[numScen[i]];
		}

		for (int j = 0; j < numCommunities; j++)
		{


			// Read comment
			fin3.getline(buffer, BUFLEN);
			cout << buffer << endl;
			sscanf(buffer, "%s", field);
			if (strcmp(field, "#") != 0) {
				cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
				cerr << "Terminating ..." << endl;
				return(1);
			}
			lineNum++;

			for (int i = 0; i < numScen[j]; i++)
			{
				fin3.getline(buffer, BUFLEN);
				sscanf(buffer, "%lf%lf%lf%lf", &myscen[j][i].prob, &myscen[j][i].contactRate, &myscen[j][i].transRate, &myscen[j][i].vacEfficacy);
				//cout << " For s = " << i << "\t prob = " << myscen[j][i].prob << "\t contactRate = " << myscen[j][i].contactRate << "\t transRate =" << myscen[j][i].transRate << " \t efficacy = " << myscen[j][i].vacEfficacy << endl;

				lineNum++;
			}
		}
		fin3.getline(buffer, BUFLEN);
		cout << buffer << endl;
		sscanf(buffer, "%s", field);
		if (strcmp(field, "#") != 0) {
			cerr << "Expecting ""#"" on line " << lineNum << " of input file " << argv[1] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}
		lineNum++;
		alpha = new double[numCommunities];
		for (int i = 0; i < numCommunities; i++)
		{
			fin3.getline(buffer, BUFLEN);
			sscanf(buffer, "%lf", &alpha[i]);
			cout << "\t alpha   = " << alpha[i] << endl;
			lineNum++;
		}
		


		fin3.getline(buffer, BUFLEN);
		sscanf(buffer, "%s", field);
		if (strcmp(field, "ENDATA") != 0) {
			cerr << "Expecting ""ENDATA"" on line " << lineNum << " of input file " << argv[3] << " but found " << field << "." << endl;
			cerr << "Terminating ..." << endl;
			return(1);
		}






		prefix = "MIPs\\DEP";
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

		////////////////////////////////////////////////////////////////////
		// Write MIP formulation to file
		//
		// NOTE: The MIP is created as an LP file and is then read in 
		// and converted to MPS format for the CORE file
		///////////////////////////////////////////////////////////////////




		fcore_d << "Min \n obj:  ";
		varcnt = 0;
		int bigM =  5000;
		int bigM2 = 5000000;

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
			index = LPcomNames[c];


			for (int n = 0; n < numHouseType; n++)
			{
				for (int v = 0; v <= n + 1; v++)
				{
					fcore_d << " + " << v * mycommunity[index - 1].typePercent[n] << " x_" << n + 1 << "_" << v << "_" << index;
					varcnt++;
					if (varcnt % 11 == 0)
					{
						fcore_d << endl;
					}

				}
				cout << "hn for community " << index << " household size " << n + 1 << " = " << mycommunity[index - 1].typePercent[n] << endl;

			}
			fcore_d << endl;

		}

	

		if (lpType == 1)
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




		cerr << " Writing schedule constraint..." << endl;

		// *** Write the constraints *** //
		fcore_d << "Subject To" << endl;
		int cnt_d = 1;
		int xcnt_d = 1; // counts the number of x decision variables

	

		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];

			for (int n = 0; n < numHouseType; n++)
			{
				fcore_d << " cx_" << cnt_d << " : ";
				for (int v = 0; v <= n + 1; v++)
				{
					fcore_d << "+ x_" << n + 1 << "_" << v << "_" << index;
				
				}
				fcore_d << " = 1" << endl;
				cnt_d++;
			}
		}


		varcnt = 0;

	
		for (int c = 0; c < numComLp; c++)
			{
					index = LPcomNames[c]; 
					
					cout << "index = " << index << endl;
					cout << "numscen = " << numScen[index - 1] << endl;

		
			for (int s = 0; s < numScen[index-1]; s++)
				{
				cout << "We are here to check 1" << endl;
				
				
				fcore_d << " cv_" << cnt_d << " : ";
				
				for (int n = 0; n < numHouseType; n++)
				{
					for (int v = 0; v <= n + 1; v++)
					{

						mycommunity[index - 1].ctrlFactor[n][v] = (myscen[index-1][s].contactRate) * (mycommunity[index - 1].typePercent[n] / avgSize[index - 1]) * ((1 - myscen[index-1][s].transRate) * (double(n) + 1 - v * myscen[index-1][s].vacEfficacy)
							+ myscen[index-1][s].transRate * pow((double(n) + 1 - v * myscen[index-1][s].vacEfficacy), 2) + myscen[index-1][s].transRate * v * myscen[index-1][s].vacEfficacy * (1 - myscen[index-1][s].vacEfficacy));
						fcore_d << " - " << mycommunity[index - 1].ctrlFactor[n][v] << "x_" << n + 1 << "_" << v << "_" << index;
						varcnt++;
						if (varcnt % 11 == 0)
						{
							fcore_d << endl;
						}
					}

				}

					fcore_d << "+ " << bigM << " z" << s + 1 << "_" << index << " >= - " << reproductionNum << endl;
					cnt_d++;
				
				
				
			}

		}

		

		


















		///09/30/2020




		for (int c = 0; c < numComLp; c++)
		{
			index = LPcomNames[c];
			cout << " index = " << index << endl;
			fcore_d << "cT_" << cnt_d << " : ";

			for (int n = 0; n < numHouseType; n++)
			{
				for (int v = 0; v <= n + 1; v++)
				{


					fcore_d << " - " << mycommunity[index - 1].householdType[n] * (v) << "x_" << n + 1 << "_" << v << "_" << index;

					varcnt++;
					if (varcnt % 11 == 0)
					{
						fcore_d << endl;
					}
				}

			}


			fcore_d << " +" << "T" << index << " =  0" << endl;
			cnt_d++;


		}
		if (lpType == 1 )
		{
			fcore_d << " cT_" << cnt_d << " : ";
			for (int c = 0; c < numComLp; c++)
			{
				index = LPcomNames[c];
				// if we input 0003, 0006 as the community that we want to include in our instance, then LpcomNames[0] is 3, and then 5
				//cout << " index = " << index << endl;
				fcore_d << " + T" << index;
			}
			fcore_d << " <=  " << totalVac << endl;
			cnt_d++;
		}
		
	

		if (lpType ==1 )
		{
			for (int i = 0; i < numComLp; i++)
			{
				index = LPcomNames[i];
				fcore_d << " cz" << cnt_d << " : ";

				for (int s = 0; s < numScen[index-1]; s++)
				{
					
						fcore_d << " + " << myscen[index -1 ][s].prob << " z" << s + 1 << "_" << index;
					

				}
				
				//fcore_d << " <= " << 1 - alpha[i] << endl;
				fcore_d << " +  alpha_e" << index << " -  alpha_s" << index  << " = " << 1 - alpha[index -1]<< endl;
				cnt_d++;
			}


		}






		if (lpType == 0 )
		{
		
		
			for (int i = 0; i < numComLp; i++)
			{
				index = LPcomNames[i];
				fcore_d << " cz" << cnt_d << " : ";

				for (int s = 0; s < numScen[index-1]; s++)
				{
					
						fcore_d << " + " << myscen[index -1][s].prob << " z" << s + 1 << "_" << index;
					
					
				}
				
				fcore_d << " <= " << 1 - alpha[index -1] << endl;
				//fcore_d << " +  alpha_"<<index << " <= " << 1 << endl;
				cnt_d++;
			}
		
			}




		varcnt = 0;

		for (int c = 0; c < numComLp; c++)
		{


			index = LPcomNames[c];

			fcore_d << " c" << cnt_d++ << ": - y_" << index;
			for (int n = 0; n < numHouseType; n++)
			{
				for (int v = 0; v <= n + 1; v++)
				{
					fcore_d <<" + " << v * mycommunity[index - 1].typePercent[n] << " x_" << n + 1 << "_" << v << "_" << index;
					varcnt++;
					if (varcnt % 11 == 0)
					{
						fcore_d << endl;
					}


				}
				//cout << "hn for community " << index << " household size " << n + 1 << " = " << mycommunity[index - 1].typePercent[n] << endl;

			}
			fcore_d<<" = 0 "  << endl;

		}





		fcore_d << "Bounds" << endl;
		


		if (lpType == 1)
		{
			
			for (int c = 0; c < numComLp; c++)
			{
				index = LPcomNames[c];
				fcore_d << "alpha_e" << index << " <= " << 1 - alpha[index - 1] << endl;
				fcore_d << "alpha_e" << index << " <= " << alpha_e[index - 1] << endl;
				fcore_d << "alpha_s" << index << " <= " << alpha_s[index - 1] << endl;
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

		///////////////////////////////////////////////////////////////
					//         Write LP file in MPS Format:                      //
					//            Close LP  file Read it into CPLEX and          //
					//            then write it as an MPS  file                  //
					///////////////////////////////////////////////////////////////

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
		// status = CPXwriteprob(env, lp, corefile_mps.c_str(), "MPS");
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


		

		cerr << "Done optimizing MIP " << corefile_lp << "." << endl << endl;
		
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
		//cout << " colnamespace = " << colnamespace << endl;

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
		resultfile = "results_";
		resultfile += to_string(lpType);
		resultfile += "_";
		resultfile += to_string(level);
		resultfile += "_";
		resultfile += to_string(alpha[0]);
		resultfile += ".csv";

		ofstream myFile(resultfile.c_str());






		cout << "Writing the csv file" << endl;

		cout << "Now writing the solutino file" << endl;
		myFile << "lptype = " << lpType << endl;
		myFile << " total vac =" << totalVac << endl;
		for (int i = 0; i < numCommunities; i++)
		{
			myFile << "alpha level = " << "," << alpha[i] << "," << "alpha_s bounds = " << "," << alpha_s[i] << "," << "alpha_e bounds =" << "," << alpha_e[i] << endl;

		}

		//ofstream myFile("solution.csv");


		int count = 0;
		for (int c = 0; c < numComLp; c++)
		{
			for (int j = 0; j < 35; j++)
			{
				myFile << mysoln[count]<<",";
				count++;
			}
			myFile << endl;
			}
			

		
	
		for (int i = 0; i < cur_ncols; i++)
		{
			if (colnames[i][0] == 'a')
			
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}

		for (int i = 0; i < cur_ncols; i++)
		{
			if (colnames[i][0] == 'T')
			{
				myFile << colnames[i] << "," << mysoln[i] << endl;
			}

		}

		for (int i = 0; i < cur_ncols; i++)
		{
			if (colnames[i][0] == 'y')
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


		for (int i = 0; i < 35; i++)
		{
			myFile << colnames[i] << ",";
		}
		myFile << endl;

		






		myFile.close();




	}
	catch (...)
	{
		cerr << "Exception thrown....something went wrong in the program ..." << endl;

	} // end catch
	return 0;
}

