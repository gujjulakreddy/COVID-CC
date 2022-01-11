# COVID-CC
## COVID-19 vaccination policies under uncertain transmission characteristics using stochastic programming

### Homogeneous model
In a homogeneous population model we assume there are no significant age-related differences in susceptibility and infectivity of individuals within the communities, regardless of age.
#### Introduction:
The files in this Homogeneous folder contain the C++ source code, header and instance input files. C++ source and header files include myCovid.cpp, stdafx.cpp and stdafx.h. In addition, the output of the this program is written to dep.lp and dep.sol which are overwritten anytime the code is running. The file dep.lp writes the dep formulation of the instance and dep.sol outputs the solution for current formulation. If you wish to save the output from running the program, dep.lp and dep.sol must each be saved as a different file after completing a fun. 

#### Installation: 
Download save and open the files. Follow directions in How_to Cplex12 to set up Microsoft Visual Studio project that links with Cplex 12. 

#### File Descriptions: 
Stdafx.h: A precompiled header file that includes standard system that are used frequently.
Stdafx.cpp: To include stdafx.h in the project.
myCovid.cpp: Main file from which the project can run. Contains steps to setup cplex environment, read instance input, steps to create the instance, solve the instance, output instance formulation and solution.
Household_Homo.txt: Input file that contains the demographic information of the seven counties in Austin area.
LpGeneration.txt: Input file that shows the parameter settings for an instance. Formulation model parameter can be modified in this file to create limited or unlimited model.
Scenario.txt: Input file that contains instance scenario parameters. Reliability level can be changed in this file.

#### USAGE: 
1) Open the MyCovid project. Then build and compile.
2) Open Scenario.txt file. The reliability level can be modified as chance prob.
3) Open the LpGeneration.txt file. To run the unlimited model, set Formulation Type to be 0. To run limited model, set Formulation Type to be 1. Then, provide the minimum coverage (MC) required for a reliability level and the fraction of MC for that reliability level. alpha_e and alpha_s bounds for each county can be modified in this file as well.
4) Open command window and change the path to where the project is.
5) In the command line, list myCovid.exe, Household_Homo.txt, LpGeneration.txt, Scenario.txt.   
6) Run program.

#### Saving Program Output:
1) Open dep.lp or dep.sol files
2) Use the save as command from the file menu to save the files as a different name.

***
***

### Hetergenoous model
In the heterogeneous population model, we assume that there are significant age-related differences in
the susceptibility and infectivity of individuals in all the communities involved.
#### Introduction
The files in Heterogeneous folder contain the C++ source code, header and instance input files. C++ source and header files include Hetero.cpp, stdafx.cpp and stdafx.h. In addition, the output of the this program is written to dep.lp and dep.sol which are overwritten anytime the code is running. The file dep.lp writes the dep formulation of the instance and dep.sol outputs the solution for current formulation. If you wish to save the output from running the program, dep.lp and dep.sol must each be saved as a different file after completing a fun. 

#### Installation: 
Download save and open the files. Follow directions in How_to Cplex12 to set up Microsoft Visual Studio project that links with Cplex 12. 

#### File Descriptions: 
Stdafx.h: A precompiled header file that includes standard system that are used frequently.
Stdafx.cpp: To include stdafx.h in the project.
Hetero.cpp: Main file from which the project can run. Contains steps to setup cplex environment, read instance input, steps to create the instance, solve the instance, output instance formulation and solution.
HeteroHousehold.txt: Input file that contains the demographic information of the seven counties in Austin area.
HeteroLp.txt: Input file that shows the parameter settings for an instance. Formulation model parameter can be modified in this file to create limited or unlimited model.
HeterScenario.txt: Input file that contains instance scenario parameters. Reliability level can be changed in this file.

#### USAGE: 
1) Open the Hetero project. Then build and compile.
2) Open HeteroScenario.txt file. The reliability level can be modified as chance prob.
3) Open the HeteroLp.txt file. To run the unlimited model, set Formulation Type to be 0. To run limited model, set Formulation Type to be 1. Then, provide the minimum coverage (MC) required for a reliability level and the fraction of MC for that reliability level. alpha_e and alpha_s bounds for each county can be modified in this file as well.
4) Open command window and change the path to where the project is.
5) In the command line, list Heter.exe, HeteroHousehold.txt, HeteroLp.txt, HeteroScenario.txt.   
6) Run program.
Saving Program Output:
===================================================================================== 
1) Open dep.lp or dep.sol files
2) Use the save as command from the file menu to save the files as a different name.

#### Contributors: Jiangyue Gong and Lewis Ntaimo

#### Prerequistes 
Read How_to_Cplex12.doc





