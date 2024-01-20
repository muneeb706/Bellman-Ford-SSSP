#include <iostream>	   // For standard input/output operations
#include <sstream>	   // For string stream operations
#include <fstream>	   // For file handling operations
#include <curl/curl.h> // For file download operations
#include <cstdio> // For popen() used in extract_tar_file()

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
			if (!line.empty() && line[0] == '%') {
				continue;
			}
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
			while (getline(check1, token, ' ') && param_no < 3)
			{
				if (param_no == 0)
				{
					param_1 = stoi(token, &sz);
				}
				else if (param_no == 1)
				{
					param_2 = stoi(token, &sz);
				}
				else if (param_no == 2)
				{
					param_3 = stoi(token, &sz);
				}
				param_no++;
			}

			// reading parameters in first line
			if (line_no == 0)
			{

				int no_of_nodes = param_1;
				int no_of_edges = param_3;
				graph->no_of_nodes = no_of_nodes;
				graph->no_of_edges = no_of_edges;

				graph->edge = (struct Edge *)malloc(graph->no_of_edges * sizeof(struct Edge));
			}
			else
			{

				// for every other line
				int source = param_1;
				int destination = param_2;
				int weight = param_3;
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

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t written = fwrite(ptr, size, nmemb, stream);
	return written;
}

void download_file(const char* url, const char* file_path) {
    std::ifstream file(file_path);
    if (file.good()) {
        // File already exists
        std::cout << "File already exists, skipping download.\n";
        return;
    }

    CURL* curl;
    FILE* fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(file_path, "wb");
        if(fp == NULL) {
            std::cout << "Could not open file for writing: " << file_path << std::endl;
            return;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}

void extract_tar_file(const char* file_path) {
    std::string command = "tar -xzf ";
    command += file_path;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return;
    }
    pclose(pipe);

    // Remove the file after extracting it
    if (std::remove(file_path) != 0) {
        std::perror("Error deleting file");
    } else {
        std::cout << "File successfully deleted.\n";
    }
}

void runBellmanFordSSSP(struct Graph *graph, int source_node_id)
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
	int source_node_id = 0;

	download_file("https://suitesparse-collection-website.herokuapp.com/MM/SNAP/higgs-twitter.tar.gz",
				  "higgs-twitter.tar.gz");
	extract_tar_file("higgs-twitter.tar.gz");
	// update this variable to provide path to input data file i.e sparse matrix
	string input_graph_path = "higgs-twitter/higgs-twitter.mtx";

	struct Graph *graph = readGraph(input_graph_path);

	auto t1 = chrono::high_resolution_clock::now();
	runBellmanFordSSSP(graph, source_node_id);
	auto t2 = chrono::high_resolution_clock::now();
	auto duration_1 = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
	cout << "Execution time (microseconds): " << duration_1 << endl;

	return 0;
}
