# COVID-CC
## COVID-19 vaccination policies under uncertain transmission characteristics using stochastic programming

### Homogeneous model
In a homogeneous population model we assume there are no significant age-related differences in susceptibility and infectivity of individuals within the communities, regardless of age.
#### Introduction:
The implementation of homogeneneous population model is called MyCovid. The program files are lcoated in the Homogeneous folder, which contains the C++ source code, header and instance input text files. The C++ source codes are myCovid.cpp and stdafx.cpp, with header file stdafx.h. The output of the program is written to dep.lp and dep.sol, which are overwritten each time the code is run. The file dep.lp writes the DEP (Deterministic Equivalent Problem) formulation of the instance, while dep.sol outputs the solution for current instance. If you wish to save the output file after running the program, you should rename dep.lp and dep.sol. 

#### Installation: 
Download, save and open the files. Follow directions in the link below, "How_to Cplex12", to set up Microsoft Visual Studio project that links with CPLEX 12 (you can use a different CPLEX version, but you have to follow the instructions for compiling and running that version). 

#### File Descriptions: 
Stdafx.h: A precompiled header file that includes standard system declarations and initializations.
Stdafx.cpp: To include stdafx.h in the project.
myCovid.cpp: Main file from which the project can run. Contains steps to setup cplex environment, read instance input, steps to create the instance, solve the instance, output instance formulation and solution.
Household_Homo.txt: Input file that contains the demographic information of the seven counties in Austin area.
LpGeneration.txt: Input file that shows the parameter settings for an instance. Formulation model parameters can be modified in this file to create limited or unlimited model.
Scenario.txt: Input file that contains instance scenario parameters. The model reliability level ("alpha") can be changed in this file.

#### USAGE: 
1) Open the MyCovid project. Then build and compile.
2) Open Scenario.txt file. The reliability level is named "chance prob" and can be modified in this file.
3) Open the LpGeneration.txt file. To run the unlimited vaccine model, set Formulation Type to 0. To run limited vaccine model, set Formulation Type to 1. Next, provide the minimum coverage (MC) required for the reliability level and the fraction of MC for that reliability level. The alpha_e and alpha_s bounds for each county are set in this file as well.
4) Open command window and change the path to where the MyCovid project is located.
5) On the command line, type: myCovid.exe Household_Homo.txt LpGeneration.txt Scenario.txt   
6) To run program, press enter.

#### Saving Program Output:
1) Open dep.lp and dep.sol files
2) Use the save as command from the file menu to save the files using a different name of your choice.

***
***

### Heterogeneous model
In the heterogeneous population model, we assume that there are significant age-related differences in
the susceptibility and infectivity of individuals in all the communities involved.
#### Introduction
The files in Heterogeneous folder contain the C++ source code, header and instance input files. The C++ source are Hetero.cpp and stdafx.cpp and header file stdafx.h. The output of the this program is written to dep.lp and dep.sol, which are overwritten each time the code is run. The file dep.lp writes the DEP (Deterministic Equivalent Problem) formulation of the instance, while dep.sol outputs the solution for current instance. If you wish to save the output file after running the program, you should rename dep.lp and dep.sol. 

#### Installation: 
Download save and open the files. Follow directions in How_to Cplex12 to set up Microsoft Visual Studio project that links with Cplex 12. 

#### File Descriptions: 
Stdafx.h: A precompiled header file that includes standard system declarations and initializations.
Stdafx.cpp: To include stdafx.h in the project.
Hetero.cpp: Main file from which the project is run. Contains steps to setup cplex environment, read instance input, steps to create the instance, solve the instance, output instance formulation and solution.
HeteroHousehold.txt: Input file that contains the demographic information of the seven counties in Austin area.
HeteroLp.txt: Input file that shows the parameter settings for an instance. Formulation model parameter can be modified in this file to create limited or unlimited model.
HeterScenario.txt: Input file that contains instance scenario parameters. The model reliability level ("alpha") can be changed in this file.

#### USAGE: 
1) Open the Hetero project. Then build and compile.
2) Open HeteroScenario.txt file. The reliability level is named "chance prob" and can be modified in this file.
3) Open the HeteroLp.txt file. To run the unlimited vaccine model, set Formulation Type to 0. To run limited vaccine model, set Formulation Type to 1. Next, provide the minimum coverage (MC) required for the reliability level and the fraction of MC for that reliability level. The alpha_e and alpha_s bounds for each county are set in this file as well.
4) Open command window and change the path to where the project is located.
5) In the command line, type: Hetero.exe HeteroHousehold.txt HeteroLp.txt HeteroScenario.txt  
6) To run program, press enter.

#### Saving Program Output:
1) Open dep.lp and dep.sol files
2) Use the save as command from the file menu to save the files using a different name of your choice.

#### Contributors: Jiangyue Gong, Krishna R. Gujjula and Lewis Ntaimo

#### Prerequisites 
Read [How_to_Cplex12.doc](https://github.com/gujjulakreddy/COVID-CC/blob/master/How_to_Cplex12.docx)





