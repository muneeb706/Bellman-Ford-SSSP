#include "graph.h"
#include <fstream>
#include <sstream>

using namespace std;

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