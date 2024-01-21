// C++ Standard Library headers
#include <iostream> // For input/output stream
#include <fstream>	// For file handling
#include <string>	// For string operations
#include <vector>	// For vector data structure
#include <chrono>	// For high resolution timing
#include <limits>	// For limits of data types

// C Standard Library headers
#include <stdio.h>	// For input/output operations
#include <stdlib.h> // For general purpose functions
#include <limits.h> // For limits of data types

// SIMD Intrinsics header
#include <arm_neon.h> // For ARM-specific SIMD intrinsics

// Custom header
#include "dataset_operations.h" // For dataset operations

using namespace std;

struct Edge
{
	int source;
	int destination;
	int weight;
};

struct Graph
{
	int no_of_nodes;
	int no_of_edges;
	struct Edge *edge;
	int *sourceIds;
	int *destIds;
	int *weights;
};

/**
 * @brief Reads a graph from a file.
 *
 * This function reads a graph from a file specified by the input path. The file should be formatted as follows:
 * - The first line should contain three integers: the number of nodes, the number of edges, and the source node.
 * - Each subsequent line should contain three integers: the source node, the destination node, and the weight of the edge.
 *
 * The function creates a new Graph structure, populates it with the data from the file, and returns a pointer to it.
 *
 * @param input_graph_path The path to the file containing the graph data.
 * @return A pointer to the newly created Graph structure.
 */
struct Graph *readGraph(string input_graph_path)
{
	struct Graph *graph = (struct Graph *)malloc(sizeof(struct Graph));

	// Open the input data file
	ifstream myfile(input_graph_path);

	if (myfile.is_open())
	{
		int line_no = 0;
		int param_1, param_2, param_3;

		while (myfile >> param_1 >> param_2 >> param_3)
		{
			// Reading parameters in first line
			if (line_no == 0)
			{
				int no_of_nodes = param_1;
				int no_of_edges = param_3;
				graph->no_of_nodes = no_of_nodes;
				graph->no_of_edges = no_of_edges;

				graph->sourceIds = new int[no_of_edges];
				graph->destIds = new int[no_of_edges];
				graph->weights = new int[no_of_edges];
			}
			else
			{
				// For every other line
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

/**
 * @brief Runs the Bellman-Ford algorithm for Single-Source Shortest Paths (SSSP) using SIMD instructions.
 *
 * This function uses the AVX-512 SIMD (Single Instruction, Multiple Data) intrinsics to perform operations
 * on multiple integers at once, which can significantly speed up the Bellman-Ford algorithm.
 *
 * @param graph A pointer to the Graph structure that represents the graph. The graph should have the following properties:
 *              - no_of_nodes: The number of nodes in the graph.
 *              - no_of_edges: The number of edges in the graph.
 *              - sourceIds: An array of integers that represent the source nodes of the edges.
 *              - destIds: An array of integers that represent the destination nodes of the edges.
 *              - weights: An array of integers that represent the weights of the edges.
 * @param source_node_id The id of the source node for the SSSP problem.
 *
 * @return void
 */
void runBellmanFordSsspSIMD(struct Graph *graph, int source_node_id)
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

		for (int j = 0; j < no_of_edges; j += 16)
		{

			// Loads 16 integers from the sourceIds, destIds, weights array into a 512-bit vector.
			// The j variable is used as an offset into the array.
			int32x4_t vs = vld1q_s32(graph->sourceIds + j);
			int32x4_t vd = vld1q_s32(graph->destIds + j);
			int32x4_t vw = vld1q_s32(graph->weights + j);

			// Gathers 16 integers from the node_distances array.
			// The indices of the integers to gather are specified by the vs and vd vectors.
			int32_t temp_s[4] = {node_distances[vgetq_lane_s32(vs, 0)], node_distances[vgetq_lane_s32(vs, 1)],
								 node_distances[vgetq_lane_s32(vs, 2)], node_distances[vgetq_lane_s32(vs, 3)]};
			int32x4_t vnds = vld1q_s32(temp_s);

			int32_t temp_d[4] = {node_distances[vgetq_lane_s32(vd, 0)], node_distances[vgetq_lane_s32(vd, 1)],
								 node_distances[vgetq_lane_s32(vd, 2)], node_distances[vgetq_lane_s32(vd, 3)]};
			int32x4_t vndd = vld1q_s32(temp_d);

			// Adds the integers in the vnds and vw vectors.
			// In other words, it computes the new distance for each node.
			int32x4_t vnewdist = vaddq_s32(vnds, vw);
			// Compare each pair of integers in the vnewdist and vndd vectors.
			// If the integer in vnewdist is less than the integer in vndd, set
			// the corresponding bit in the mask to 1, otherwise set it to 0.
			uint32x4_t mask = vcltq_s32(vnewdist, vndd);
			// For each integer in the vd vector, if the corresponding bit in the mask
			// is set to 1, store the corresponding integer from the vnewdist vector in
			// the node_distances array at the index specified by the integer in the vd vector.
			int32_t temp[4];
			vst1q_s32(temp, vnewdist);

			if (mask[0]) {
				node_distances[vgetq_lane_s32(vd, 0)] = temp[0];
			}
			if (mask[1]) {
				node_distances[vgetq_lane_s32(vd, 1)] = temp[1];
			}
			if (mask[2]) {
				node_distances[vgetq_lane_s32(vd, 2)] = temp[2];
			}
			if (mask[3]) {
				node_distances[vgetq_lane_s32(vd, 3)] = temp[3];
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
		int node_1 = graph->sourceIds[i];
		int node_2 = graph->destIds[i];
		int weight = graph->weights[i];

		if (node_distances[node_1] + weight < node_distances[node_2])
			cout << "Negative weight edge cycle is present in the graph\n";
	}

	cout << "Following are the outputs for SIMD bellman ford without tilling" << endl;
	cout << "Shortest Distance from node 0 to 100 is: ";
	cout << node_distances[100] << endl;

	return;
}

/**
 * @brief Runs the Bellman-Ford algorithm for Single-Source Shortest Paths (SSSP) using SIMD instructions and tiling.
 *
 * This function uses the AVX-512 SIMD (Single Instruction, Multiple Data) intrinsics to perform operations on multiple
 * integers at once, which can significantly speed up the Bellman-Ford algorithm. It also uses tiling to optimize
 * memory access patterns and improve cache efficiency.
 *
 * @param graph A pointer to the Graph structure that represents the graph. The graph should have the following properties:
 *              - no_of_nodes: The number of nodes in the graph.
 *              - no_of_edges: The number of edges in the graph.
 *              - sourceIds: An array of integers that represent the source nodes of the edges.
 *              - destIds: An array of integers that represent the destination nodes of the edges.
 *              - weights: An array of integers that represent the weights of the edges.
 * @param source_node_id The id of the source node for the SSSP problem.
 *
 * @return void
 *
 * @note This function prints the presence of a negative weight edge cycle in the graph if it exists.
 * It also prints the shortest distance from the source node to node 100.
 */
void runBellmanFordSsspSIMDTilling(struct Graph *graph, int source_node_id)
{
	int no_of_nodes = graph->no_of_nodes;
	int no_of_edges = graph->no_of_edges;

	cout << "Tilling process is started. " << endl;

	int tile_size = 1024;
	int no_of_tiles = ceil(no_of_nodes / tile_size);

	vector<tuple<int, int, float>> coo_tiles[no_of_tiles];
	for (int i = 0; i < no_of_edges; i++)
	{
		int tile_id = graph->destIds[i] / tile_size;
		// Add the source node, destination node, and weight of the current edge
		// to the corresponding tile in the coo_tiles vector
		coo_tiles[tile_id].push_back(make_tuple(graph->sourceIds[i], graph->destIds[i], graph->weights[i]));
	}
	// Initialize a counter to keep track of the current edge index
	int count = 0;
	// Reorganizing the edges in the graph based on the tiles
	// This is done to optimize memory access patterns and improve cache efficiency during
	// the execution of the Bellman-Ford algorithm.
	for (int i = 0; i < no_of_tiles; i++)
	{
		for (auto &t : coo_tiles[i])
		{
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

		for (int j = 0; j < no_of_edges; j += 16)
		{

			// Load data into NEON registers
			int32x4_t vs = vld1q_s32(graph->sourceIds + j);
			int32x4_t vd = vld1q_s32(graph->destIds + j);
			int32x4_t vw = vld1q_s32(graph->weights + j);

			// Emulate gather operations
			int32_t temp_s[4] = {node_distances[vgetq_lane_s32(vs, 0)], node_distances[vgetq_lane_s32(vs, 1)],
								 node_distances[vgetq_lane_s32(vs, 2)], node_distances[vgetq_lane_s32(vs, 3)]};
			int32x4_t vnds = vld1q_s32(temp_s);

			int32_t temp_d[4] = {node_distances[vgetq_lane_s32(vd, 0)], node_distances[vgetq_lane_s32(vd, 1)],
								 node_distances[vgetq_lane_s32(vd, 2)], node_distances[vgetq_lane_s32(vd, 3)]};
			int32x4_t vndd = vld1q_s32(temp_d);

			// Add NEON registers
			int32x4_t vnewdist = vaddq_s32(vnds, vw);

			// Compare NEON registers
			uint32x4_t mask = vcltq_s32(vnewdist, vndd);

			// Emulate scatter operation
			int32_t temp[4];
			vst1q_s32(temp, vnewdist);

			if (mask[0]) {
				node_distances[vgetq_lane_s32(vd, 0)] = temp[0];
			}
			if (mask[1]) {
				node_distances[vgetq_lane_s32(vd, 1)] = temp[1];
			}
			if (mask[2]) {
				node_distances[vgetq_lane_s32(vd, 2)] = temp[2];
			}
			if (mask[3]) {
				node_distances[vgetq_lane_s32(vd, 3)] = temp[3];
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
		int node_1 = graph->sourceIds[i];
		int node_2 = graph->destIds[i];
		int weight = graph->weights[i];

		if (node_distances[node_1] + weight < node_distances[node_2])
			cout << "Negative weight edge cycle is present in the graph\n";
	}

	cout << "Following are the outputs for SIMD bellman ford with tilling" << endl;
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

	struct Graph *graph = readGraph(input_graph_path);

	auto t1 = chrono::high_resolution_clock::now();
	runBellmanFordSsspSIMD(graph, source_node_id);
	auto t2 = chrono::high_resolution_clock::now();
	auto duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Execution time (microseconds): " << duration_1 << endl;

	t1 = chrono::high_resolution_clock::now();
	runBellmanFordSsspSIMDTilling(graph, source_node_id);
	t2 = chrono::high_resolution_clock::now();
	duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Execution time (microseconds): " << duration_1 << endl;

	return 0;
}
