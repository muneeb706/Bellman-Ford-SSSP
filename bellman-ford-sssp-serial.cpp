#include "graph.h" 	   // For graph data structure
#include "dataset_operations.h" // For dataset operations

using namespace std;

/**
 * @brief Runs the Bellman-Ford algorithm to find the shortest path from a source node to all other nodes in a graph.
 *
 * This function implements the Bellman-Ford algorithm for finding the shortest path from a source node to all other nodes in a graph.
 * The graph and the source node ID are provided as arguments. The graph is represented as an adjacency list.
 * The function initializes the distance to all nodes as infinity, except for the source node which is set to zero.
 * It then relaxes the edges of the graph in a loop that runs for (number of nodes - 1) times.
 * If in any iteration no edge is relaxed, it breaks out of the loop.
 * Finally, it checks for the presence of negative weight cycles in the graph.
 * If such a cycle is found, it prints a message to the console.
 * The function does not return a value; instead, it prints the shortest distance from the source node to node 100.
 *
 * @param graph A pointer to the graph on which the algorithm should be run.
 * @param source_node_id The ID of the source node from which the shortest paths should be found.
 */
void runBellmanFordSSSP(struct Graph *graph, int source_node_id)
{
	int no_of_nodes = graph->no_of_nodes;
	int no_of_edges = graph->no_of_edges;

	// distance of node 10 will be at index 10.
	int node_distances[no_of_nodes];

	// setting initial distances to infinity
	for (int i = 0; i < no_of_nodes; i++)
		node_distances[i] = numeric_limits<int>::max() - 100;

	// setting the distance of the source node to zero
	node_distances[source_node_id] = 0;

	for (int i = 0; i < no_of_nodes - 1; i++)
	{
		bool relaxed = false;

		for (int j = 0; j < no_of_edges; j++)
		{
			int node_1 = graph->edge[j].source;
			int node_2 = graph->edge[j].destination;
			int weight = graph->edge[j].weight;

			if (node_distances[node_1] + weight < node_distances[node_2])
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

	// checking for negative weight cycle
	for (int i = 0; i < no_of_edges; i++)
	{
		int node_1 = graph->edge[i].source;
		int node_2 = graph->edge[i].destination;
		int weight = graph->edge[i].weight;

		if (node_distances[node_1] + weight < node_distances[node_2])
			cout << "Negative weight edge cycle is present in the graph\n";
	}

	cout << "Shortest Distance from node 0 to 100 is: ";
	cout << node_distances[100] << endl;

	return;
}

int main()
{

	download_file("https://suitesparse-collection-website.herokuapp.com/MM/SNAP/higgs-twitter.tar.gz",
				  "higgs-twitter.tar.gz");
	extract_tar_file("higgs-twitter.tar.gz");
	// update this variable to provide path to input data file i.e sparse matrix
	string input_graph_path = "higgs-twitter/higgs-twitter.mtx";

	int source_node_id = 0;

	struct Graph *graph = readGraph(input_graph_path);

	auto t1 = chrono::high_resolution_clock::now();

	runBellmanFordSSSP(graph, source_node_id);

	auto t2 = chrono::high_resolution_clock::now();

	auto duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Bellman Ford Serial Execution time (microseconds): " << duration_1 << endl;

	return 0;
}
