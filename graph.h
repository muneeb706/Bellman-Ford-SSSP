#ifndef GRAPH_H
#define GRAPH_H

#include <string>

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
    Edge *edge;
};

// Function declaration
Graph* readGraph(std::string input_graph_path);

#endif // GRAPH_H