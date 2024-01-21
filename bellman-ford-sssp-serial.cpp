#include <iostream>	   // For standard input/output operations
#include <sstream>	   // For string stream operations
#include <fstream>	   // For file handling operations
#include <curl/curl.h> // For file download operations
#include <cstdio>	   // For popen() used in extract_tar_file()

using namespace std;

struct Graph
{
	int no_of_nodes;
	int no_of_edges;
	struct Edge *edge;
};

struct Edge
{
	int source;
	int destination;
	int weight;
};

/**
 * @brief Writes data to a stream.
 *
 * This function is a callback used by libcurl to write data to a stream. It is typically
 * used when downloading a file. The data to be written is provided by libcurl and the
 * stream to which it should be written is provided as an argument.
 *
 * @param ptr A pointer to the data to be written.
 * @param size The size of each unit of data to be written.
 * @param nmemb The number of units of data to be written.
 * @param stream The stream to which the data should be written.
 * @return The number of bytes actually written.
 */
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

/**
 * @brief Downloads a file from a given URL and saves it to a specified path.
 *
 * This function uses libcurl to download a file. If the file already exists at the
 * specified path, the download is skipped. If the file does not exist, it is created
 * and the content from the URL is written to it. If the download fails for any reason,
 * an error message is printed to the console.
 *
 * @param url The URL of the file to download.
 * @param file_path The path where the downloaded file should be saved.
 */
void download_file(const char *url, const char *file_path)
{
	std::ifstream file(file_path);
	if (file.good())
	{
		// File already exists
		std::cout << "Tar File already exists, skipping download.\n";
		return;
	}

	CURL *curl;
	FILE *fp;
	CURLcode res;

	curl = curl_easy_init();
	if (curl)
	{
		fp = fopen(file_path, "wb");
		if (fp == NULL)
		{
			std::cout << "Could not open file for writing: " << file_path << std::endl;
			return;
		}
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
	}
}

/**
 * @brief Extracts a tar file to a specified directory.
 *
 * This function uses the libarchive library to extract a tar file. The tar file and the
 * directory to which it should be extracted are provided as arguments. If the extraction
 * fails for any reason, an error message is printed to the console.
 *
 * @param tar_file_path The path to the tar file to be extracted.
 * @param extract_dir_path The path to the directory where the tar file should be extracted.
 */
void extract_tar_file(const char *file_path)
{
	std::string command = "tar -xzf ";
	command += file_path;

	FILE *pipe = popen(command.c_str(), "r");
	if (!pipe)
	{
		return;
	}
	pclose(pipe);

	// Remove the file after extracting it
	if (std::remove(file_path) != 0)
	{
		std::perror("Error deleting file");
	}
	else
	{
		std::cout << "Tar File successfully deleted.\n";
	}
}

/**
 * @brief Reads a graph from a file and returns a pointer to the graph.
 *
 * This function reads a graph from a file whose path is provided as an argument.
 * The graph is represented as an adjacency matrix. The first line of the file should
 * contain three integers: the number of nodes, the number of edges, and the weight of the edge.
 * Each subsequent line should contain three integers: the source node, the destination node,
 * and the weight of the edge between them.
 *
 * @param input_graph_path The path to the file containing the graph data.
 * @return A pointer to the graph read from the file.
 */
struct Graph *readGraph(string input_graph_path)
{
	struct Graph *graph = (struct Graph *)malloc(sizeof(struct Graph));

	// reading input data file
	ifstream myfile(input_graph_path);
	// variable to get content of the input data file line by line
	string line;

	int row_index = -1, col_index = -1, value;

	if (myfile.is_open())
	{
		int line_no = 0;
		while (getline(myfile, line))
		{
			// Skip comments
			if (!line.empty() && line[0] == '%')
			{
				continue;
			}
			// value of parameter will be stored in this variable.
			string token;
			// alias of size_t
			std::string::size_type sz;
			// stringstream class check1
			stringstream check1(line);
			int param_no = 0;
			// variable to store three parameters in a line
			int params[3];
			// Tokenizing each line to get parameter values w.r.t. space ' '
			while (getline(check1, token, ' ') && param_no < 3)
			{
				params[param_no++] = stoi(token, &sz);
			}

			// reading parameters in first line
			if (line_no == 0)
			{

				int no_of_nodes = params[0];
				int no_of_edges = params[2];
				graph->no_of_nodes = no_of_nodes;
				graph->no_of_edges = no_of_edges;

				graph->edge = (struct Edge *)malloc(graph->no_of_edges * sizeof(struct Edge));
			}
			else
			{

				// for every other line
				int source = params[0];
				int destination = params[1];
				int weight = params[2];
				graph->edge[line_no - 1].source = source;
				graph->edge[line_no - 1].destination = destination;
				graph->edge[line_no - 1].weight = weight;
			}

			line_no++;
		}
		myfile.close();
	}

	return graph;
}

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
	cout << "Execution time (microseconds): " << duration_1 << endl;

	return 0;
}
