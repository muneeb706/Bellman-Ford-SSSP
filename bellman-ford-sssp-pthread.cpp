#include <sstream>				// For string stream operations
#include <pthread.h>			// For multithreading operations
#include "graph.h"				// For graph data structure
#include "dataset_operations.h" // For dataset operations

using namespace std;

// global variables that all threads can access and modify it.

struct Graph *graph;
// distance of node 10 will be at index 10.
vector<long> node_distances;
int num_threads;
int bsize;
int no_of_nodes;
int no_of_edges;
pthread_t *threads;

void *relaxEdges(void *tn)
{

	int tid = (intptr_t)tn;

	for (int i = 0; i < no_of_nodes - 1; i++)
	{
		bool relaxed = false;

		for (int j = tid * bsize; j < (tid + 1) * bsize && j < no_of_edges; j++)
		{
			int node_1 = graph->edge[j].source;
			int node_2 = graph->edge[j].destination;
			int weight = graph->edge[j].weight;

			// If the shortest distance to the source node (node_1) is not infinity
			// (which means the node has been visited)
			// and the distance from the source node to the destination node (node_2)
			// through the current edge is less than
			// the current shortest distance to the destination node,
			// then update the shortest distance to the destination node and mark
			// that a relaxation has occurred in this iteration.
			if (node_distances[node_1] != numeric_limits<int>::max() &&
				node_distances[node_1] + weight < node_distances[node_2])
			{
				node_distances[node_2] = node_distances[node_1] + weight;
				relaxed = true;
			}
		}

		// if no edge was relaxed in this iteration
		// then stop iterating
		if (!relaxed)
		{
			break;
		}
	}

	pthread_exit(NULL);
}

void runBellmanFordSSSP(int source_node_id)
{

	no_of_nodes = graph->no_of_nodes;
	no_of_edges = graph->no_of_edges;

	// distance of node 10 will be at index 10.
	node_distances = vector<long>(no_of_nodes);

	threads = new pthread_t[num_threads];

	bsize = no_of_edges / num_threads;

	// setting initial distances
	for (int i = 0; i < no_of_nodes; i++)
		node_distances[i] = numeric_limits<int>::max() - 100;

	node_distances[source_node_id] = 0;

	for (int tn = 0; tn < num_threads; tn++)
		// For each thread, create a new thread that executes the relaxEdges function.
		// The thread ID is passed as an argument to the function.
		pthread_create(&threads[tn], NULL, relaxEdges, (void *)(intptr_t)tn);

	for (int tn = 0; tn < num_threads; tn++)
		// Wait for each thread to finish execution.
		// This ensures that the main thread (which is executing this code)
		// will not continue until all the spawned threads have finished.
		pthread_join(threads[tn], NULL);

	// checking for negative weight cycle
	for (int i = 0; i < no_of_edges; i++)
	{
		int node_1 = graph->edge[i].source;
		int node_2 = graph->edge[i].destination;
		int weight = graph->edge[i].weight;

		// If the shortest distance to the source node plus the weight of the edge
		// is less than the shortest distance to the destination node,
		// then we have found a shorter path to the destination node that includes an extra edge.
		// However, since we have already performed all the necessary relaxations,
		// this can only happen if there is a negative weight cycle in the graph.
		if (node_distances[node_1] != numeric_limits<int>::max() &&
			node_distances[node_1] + weight < node_distances[node_2])
			cout << "Negative weight edge cycle is present in the graph\n";
	}

	cout << "Shortest Distance from node 0 to 100 is: ";
	cout << node_distances[100] << endl;

	return;
}

int main()
{
	int source_node_id = 0;

	download_file("https://suitesparse-collection-website.herokuapp.com/MM/SNAP/higgs-twitter.tar.gz",
				  "higgs-twitter.tar.gz");
	extract_tar_file("higgs-twitter.tar.gz");
	// update this variable to provide path to input data file i.e sparse matrix
	string input_graph_path = "higgs-twitter/higgs-twitter.mtx";

	graph = readGraph(input_graph_path);

	cout << endl
		 << "No. of threads = 2" << endl;
	num_threads = 2;
	auto t1 = chrono::high_resolution_clock::now();
	runBellmanFordSSSP(source_node_id);
	auto t2 = chrono::high_resolution_clock::now();
	auto duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Execution time (microseconds): " << duration_1 << endl;

	cout << endl
		 << "No. of threads = 4" << endl;
	num_threads = 4;
	t1 = chrono::high_resolution_clock::now();
	runBellmanFordSSSP(source_node_id);
	t2 = chrono::high_resolution_clock::now();
	duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Execution time (microseconds): " << duration_1 << endl;

	cout << endl
		 << "No. of threads = 8" << endl;
	num_threads = 8;
	t1 = chrono::high_resolution_clock::now();
	runBellmanFordSSSP(source_node_id);
	t2 = chrono::high_resolution_clock::now();
	duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Execution time (microseconds): " << duration_1 << endl;

	return 0;
}
