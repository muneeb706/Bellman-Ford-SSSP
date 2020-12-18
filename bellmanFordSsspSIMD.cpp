#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fstream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <chrono>
#include <limits>
#include <immintrin.h>

using namespace std;
 
struct Graph
{
    int no_of_nodes;
    int no_of_edges;
    struct Edge* edge;
    int *sourceIds;
    int *destIds;
    int *weights;
};
 


struct Graph* readGraph(string input_graph_path)
{
	struct Graph* graph = (struct Graph*) malloc( sizeof(struct Graph));
	
	// reading input data file
	ifstream myfile (input_graph_path);
	// variable to get content of the input data file line by line
	string line;
	
	int row_index = -1, col_index = -1, value;
	
	if (myfile.is_open())
	{
		int line_no = 0;
		while ( getline (myfile,line) )
	    {
	    	// value of parameter will be stored in this variable.
	    	string token;
	    	// alias of size_t
			std::string::size_type sz;   
			// stringstream class check1 
			stringstream check1(line); 
	    	int param_no = 0;
			// variables to store three parameters in each line separately
			int param_1, param_2, param_3;
			// Tokenizing each line to get parameter values w.r.t. space ' ' 
			while(getline(check1, token, ' ') && param_no < 3) 
			{ 
			    if (param_no == 0) {
			    	param_1 = stoi(token,&sz);
				} else if (param_no == 1) {
					param_2 = stoi(token,&sz);
				} else if (param_no == 2) {
					param_3 = stoi(token,&sz);
				}
			    param_no++;
			}
			
			// reading parameters in first line	
	    	if (line_no == 0) {
				
				int no_of_nodes = param_1;
				int no_of_edges = param_3;
				graph->no_of_nodes = no_of_nodes;
    			graph->no_of_edges = no_of_edges;
    			
    			graph->sourceIds = new int[no_of_edges];
    			graph->destIds = new int[no_of_edges];
    			graph->weights = new int[no_of_edges];
    			
			} else {
				
				// for every other line
				int source = param_1;
				int destination = param_2;
				int weight = param_3;
				graph->sourceIds[line_no - 1] = source;
        		graph->destIds[line_no - 1] = destination;
        		graph->weights[line_no - 1] = weight;		
			
			}
	      
	      	line_no++;
	    }
	    myfile.close();
	}
	
 
    
	
    return graph;
}

void runBellmanFordSsspSerial(struct Graph* graph, int source_node_id)
{
    int no_of_nodes = graph->no_of_nodes;
    int no_of_edges = graph->no_of_edges;
 
 	// distance of node 10 will be at index 10.
    int node_distances[no_of_nodes];
 
    // setting initial distances
    for (int i = 0; i < no_of_nodes; i++)
        node_distances[i] = numeric_limits<int>::max() - 100;
 
    node_distances[source_node_id] = 0;
 
    for (int i = 0; i < no_of_nodes - 1; i++)
    {
    	bool relaxed = false;
    
	    for (int j = 0; j < no_of_edges; j++)
        {
            int node_1 = graph->sourceIds[j];
            int node_2 = graph->destIds[j];
            int weight = graph->weights[j];
 
            if (node_distances[node_1] + weight < node_distances[node_2]) {
                node_distances[node_2] = node_distances[node_1] + weight;
            	relaxed = true;
			}   
        }
        
        // if no edge was relaxed in this iteration
        // then stop iterating
        if (!relaxed) {
        	break;
		}
    }
 
    // checking for negative weight cycle
	for (int i = 0; i < no_of_edges; i++)
    {
        int node_1 = graph->sourceIds[i];
        int node_2 = graph->destIds[i];
        int weight = graph->weights[i];
 
        if (node_distances[node_1] + weight < node_distances[node_2])
            cout << "Negative weight edge cycle is present in the graph\n";
    }

 	cout << "Following are the outputs for serial bellman ford"<<endl;
 	cout << "Shortest Distance from node 0 to 100 is: ";
    cout << node_distances[100] <<endl;
	
 
    return;
}
 
void runBellmanFordSsspSIMD(struct Graph* graph, int source_node_id)
{
    int no_of_nodes = graph->no_of_nodes;
    int no_of_edges = graph->no_of_edges;
 
 	// distance of node 10 will be at index 10.
    int node_distances[no_of_nodes];
 
    // setting initial distances
    for (int i = 0; i < no_of_nodes; i++)
        node_distances[i] = numeric_limits<int>::max() - 100;
 
    node_distances[source_node_id] = 0;
 
    for (int i = 0; i < no_of_nodes - 1; i++)
    {
    	bool relaxed = false;
    
	    for (int j = 0; j < no_of_edges; j+=16)
        {
            
            __m512i vs = _mm512_load_epi32(graph->sourceIds + j);
			__m512i vd = _mm512_load_epi32(graph->destIds + j);
            __m512i vw = _mm512_load_epi32(graph->weights+j);
			
			__m512i vnds = _mm512_i32gather_epi32(vs,node_distances,4); 
            __m512i vndd = _mm512_i32gather_epi32(vd,node_distances,4);
            __m512i vnewdist = _mm512_add_epi32(vnds, vw);
            
			__mmask16 mask = _mm512_cmplt_epi32_mask(vnewdist,vndd);
			
			//_mm512_cmpeq_epi32_mask(mask, vnewdist)
			
			_mm512_mask_i32scatter_epi32(node_distances,mask,vd,vnewdist,4); 
        }
        
        // if no edge was relaxed in this iteration
        // then stop iterating
        if (!relaxed) {
        	break;
		}

    }
 
    // checking for negative weight cycle
	for (int i = 0; i < no_of_edges; i++)
    {
    	int node_1 = graph->sourceIds[i];
        int node_2 = graph->destIds[i];
        int weight = graph->weights[i];
        
		if (node_distances[node_1] + weight < node_distances[node_2])
            cout << "Negative weight edge cycle is present in the graph\n";
    }
 
  	cout << "Following are the outputs for SIMD bellman ford without tilling"<<endl;
 	cout << "Shortest Distance from node 0 to 100 is: ";
    cout << node_distances[100] <<endl;
	
    return;
}

void runBellmanFordSsspSIMDTilling(struct Graph* graph, int source_node_id)
{
    int no_of_nodes = graph->no_of_nodes;
    int no_of_edges = graph->no_of_edges;
 
 	cout << "Tilling process is started. " << endl;
 	
 	// tilling using solution of homework 4
 	int tile_size = 1024;
 	int no_of_tiles = ceil(no_of_nodes / tile_size);
 	
 	vector<tuple<int, int, float>> coo_tiles[no_of_tiles];
	for (int i=0; i<no_of_edges; i++) {
		int tile_id = graph->destIds[i] / tile_size;
		coo_tiles[tile_id].push_back(make_tuple(graph->sourceIds[i], graph->destIds[i], graph->weights[i]));
	}
	int count = 0;
	for (int i=0; i<no_of_tiles; i++) {
		for (auto &t: coo_tiles[i]) {
			graph->sourceIds[count] = get<0>(t); 
			graph->destIds[count] = get<1>(t);
			graph->weights[count] = get<2>(t);
			count++;
		}
	}
	cout << "Tilling process is completed. " << endl;
 	
 	// distance of node 10 will be at index 10.
    int node_distances[no_of_nodes];
 
    // setting initial distances
    for (int i = 0; i < no_of_nodes; i++)
        node_distances[i] = numeric_limits<int>::max() - 100;
 
    node_distances[source_node_id] = 0;
 
    for (int i = 0; i < no_of_nodes - 1; i++)
    {
    	bool relaxed = false;
    
	    for (int j = 0; j < no_of_edges; j+=16)
        {
            
            __m512i vs = _mm512_load_epi32(graph->sourceIds + j);
			__m512i vd = _mm512_load_epi32(graph->destIds + j);
            __m512i vw = _mm512_load_epi32(graph->weights+j);
			
			__m512i vnds = _mm512_i32gather_epi32(vs,node_distances,4); 
            __m512i vndd = _mm512_i32gather_epi32(vd,node_distances,4);
            __m512i vnewdist = _mm512_add_epi32(vnds, vw);
            
			__mmask16 mask = _mm512_cmplt_epi32_mask(vnewdist,vndd);
			
			//_mm512_cmpeq_epi32_mask(mask, vnewdist)
			
			_mm512_mask_i32scatter_epi32(node_distances,mask,vd,vnewdist,4); 
        }
        
        // if no edge was relaxed in this iteration
        // then stop iterating
        if (!relaxed) {
        	break;
		}

    }
 
    // checking for negative weight cycle
	for (int i = 0; i < no_of_edges; i++)
    {
    	int node_1 = graph->sourceIds[i];
        int node_2 = graph->destIds[i];
        int weight = graph->weights[i];
        
		if (node_distances[node_1] + weight < node_distances[node_2])
            cout << "Negative weight edge cycle is present in the graph\n";
    }
 
  	cout << "Following are the outputs for SIMD bellman ford with tilling"<<endl;
 	cout << "Shortest Distance from node 0 to 100 is: ";
    cout << node_distances[100] <<endl;
	
    return;
}

int main()
{
    int source_node_id = 0;
	 
	// update this variable to provide path to input data file i.e sparse matrix	
	string input_graph_path = "higgs-twitter.mtx";

	struct Graph* graph = readGraph(input_graph_path);

 	auto t1 = chrono::high_resolution_clock::now();
    runBellmanFordSsspSerial(graph, source_node_id);
    auto t2 = chrono::high_resolution_clock::now();
    auto duration_1 = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();
    cout <<"Execution time (microseconds): " << duration_1 <<endl;

 
 	t1 = chrono::high_resolution_clock::now();
    runBellmanFordSsspSIMD(graph, source_node_id);
    t2 = chrono::high_resolution_clock::now();
    duration_1 = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();
    cout <<"Execution time (microseconds): " << duration_1 <<endl;

 	t1 = chrono::high_resolution_clock::now();
    runBellmanFordSsspSIMDTilling(graph, source_node_id);
    t2 = chrono::high_resolution_clock::now();
    duration_1 = chrono::duration_cast<chrono::microseconds>( t2 - t1 ).count();
    cout <<"Execution time (microseconds): " << duration_1 <<endl;

    return 0;
}

