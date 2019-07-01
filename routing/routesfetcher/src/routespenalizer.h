#ifndef MAIN_ROUTESPENALIZER_H
#define MAIN_ROUTESPENALIZER_H

#include <KDTree.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <json11.hpp>
#include <arlib/esx.hpp>
#include <arlib/graph_utils.hpp>
#include <arlib/multi_predecessor_map.hpp>
#include <arlib/onepass_plus.hpp>
#include <arlib/path.hpp>
#include <arlib/penalty.hpp>
#include <arlib/routing_kernels/types.hpp>
#include <arlib/terminators.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>
#include <float.h>
#include <chrono>
#include "utils.hpp"
#include <filesystem>

template <typename Graph>
void penalize_edges(Graph &g, Graph &g_pen, int perc, int multi){
    // get the property map for vertex indices
    typedef typename boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    //copy_graph(g, g_pen);
    copy_graph(g, g_pen);

    cout << num_edges(g_pen) << endl;
    int m_edges = num_edges(g_pen) / 100.0 * perc;
    cout << m_edges << endl;

    IndexMap index = get(boost::vertex_index, g_pen);
    for (boost::tie(ei, ei_end) = edges(g_pen); ei != ei_end; ++ei) {

        std::pair<Edge, bool> ed = boost::edge(index[source(*ei, g_pen)], index[target(*ei, g_pen)], g_pen);
        //float weight = get(boost::edge_weight_t(), g_pen, ed.first);
        //cout << "Weight: " << weight;
        get(boost::edge_weight_t(), g_pen, ed.first) *= multi;
        //float new_weight = get(boost::edge_weight_t(), g_pen, ed.first);
        //cout << ", new Weight: " << new_weight << endl;
    }

    //cout << num_edges(g) << endl;

	cout << "Percentage: " << perc << endl;
    cout << "Multiplier: " << multi << endl;
}
#endif //MAIN_ROUTESPENALIZER_H