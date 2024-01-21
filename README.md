# Bellman-Ford-SSSP
This repository contains implementations of the Bellman-Ford algorithm for Single Source Shortest Path (SSSP) problem in three different ways:

1. Serial implementation (`bellman-ford-sssp-serial.cpp`)
2. Parallel implementation using pthreads (`bellman-ford-sssp-pthread.cpp`)
3. Parallel implementation using SIMD and Tiling (`bellman-ford-sssp-simd.cpp`)

## Dataset
The program automatically downloads and uses the `higgs-twitter.mtx` data file for its operations. This dataset is part of the [Higgs Twitter dataset](https://suitesparse-collection-website.herokuapp.com/MM/SNAP/higgs-twitter.tar.gz), which captures the spread of news about the discovery of a new particle with the features of the Higgs boson on 4th July 2012.

## Compilation and Execution
The code can be compiled and executed using the `g++` compiler for the serial and parallel implementations. However, for the SIMD vectorized version, the `ARM architecture` is needed. 

Please follow the instructions below to compile and execute the programs:

### For Serial Program

#### Compile
```bash
g++ bellman-ford-sssp-serial.cpp graph.cpp dataset_operations.cpp -o bellman-ford-sssp-serial -std=c++20 -lcurl
```
#### Execute
```bash
./bellman-ford-sssp-serial 
```

### For Parallel Program

#### Compile
```bash
g++ bellman-ford-sssp-pthread.cpp graph.cpp dataset_operations.cpp -o bellman-ford-sssp-pthread -std=c++20 -lpthread -lcurl
```
#### Execute
```bash
./bellman-ford-sssp-pthread
```

### For SIMD Vectorization Program

The SIMD vectorized version of the program is designed to run on ARM architecture, specifically using ARM NEON intrinsics for SIMD vectorization. 

#### Compile
```bash
g++ bellman-ford-sssp-simd.cpp dataset_operations.cpp -o bellman-ford-sssp-simd -march=armv8-a -mfpu=neon -std=c++20 -lcurl
```
#### Execute
```bash
./bellman-ford-sssp-simd
```