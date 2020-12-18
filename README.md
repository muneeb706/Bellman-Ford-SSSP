# Bellman-Ford-SSSP
In this project, serial, parallel and SIMD (Single Instruction Multiple Data) vectorized version of bellman ford algorithm for single source shortest path is implemented.

## Dataset
Experiment is done on the higgs-twitter.mtx data file, this file can be downloaded from [higgs-twitter-dataset](https://suitesparse-collection-website.herokuapp.com/MM/SNAP/higgs-twitter.tar.gz). After downloading the compressed folder search for higgs-twitter.mtx data file and put the file in the same folder where code files are placed.

## Compilation and Execution
Code can be compiled and executed using g++ compiler for parallel and serial iplmentation but for SIMD vectorized version intel module is needed. Please enter following commands in the terminal after navigating to folder where code files are placed to compile.

### For Serial Program

#### Compile
g++ -o bellman-ford-sssp-serial bellman-ford-sssp-serial.cpp -std=c++11
#### Execute
./bellman-ford-sssp-serial

### For Parallel Program

#### Compile
g++ -o bellman-ford-sssp-pthread bellman-ford-sssp-pthread.cpp -std=c++11 -lpthread
#### Execute
./bellman-ford-sssp-pthread


### For SIMD Vectorization Program

#### Compile
module load intel
icc -std=c++11 bellmanFordSsspSIMD.cpp
#### Execute
./a.out
