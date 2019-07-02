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

// Rect Struct
struct rectangle {
    float min_lat;
    float max_lat;
    float min_long;
    float max_long;
    int multi;
};
// Rect List


rectangle fillRect(float min_lat, float max_lat, float min_long, float max_long, int multi);
bool checkInput(rectangle rect);
bool vertexInRect(rectangle rect, float lat, float lon);

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

template <typename Graph>
void add_penalization_rect(Graph &g, Graph &g_pen, int multi, float min_lat, float max_lat, float min_long, float max_long){
    typedef typename boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    typename Graph::vertex_iterator v, vend;

    if(boost::num_vertices(g_pen) == 0)
        copy_graph(g, g_pen);

    int count_edges = 0;
    IndexMap index = get(boost::vertex_index, g_pen);
    for (boost::tie(ei, ei_end) = edges(g_pen); ei != ei_end; ++ei) {
        std::pair<Edge, bool> ed = boost::edge(index[source(*ei, g_pen)], index[target(*ei, g_pen)], g_pen);

        float source_lat = g_pen[source(*ei, g_pen)].lat;
        float target_lat = g_pen[target(*ei, g_pen)].lat;

        float source_lon = g_pen[source(*ei, g_pen)].lon;
        float target_lon = g_pen[target(*ei, g_pen)].lon;
        rectangle tmp_rect = fillRect(min_lat, max_lat, min_long, max_long, multi);

        if(vertexInRect(tmp_rect, source_lat, source_lon) || vertexInRect(tmp_rect, target_lat, target_lon)){
            //float weight = get(boost::edge_weight_t(), g_pen, ed.first);
            //cout << "source_lat: " << source_lat << " source_lon: " << source_lon;
            //cout << " target_lat: " << target_lat << " target_lon: " << target_lon;
            //cout << " Weight: " << weight;
            get(boost::edge_weight_t(), g_pen, ed.first) *= multi;
            //float new_weight = get(boost::edge_weight_t(), g_pen, ed.first);
            //cout << ", new Weight: " << new_weight << endl;
            count_edges++;
        }
    }
    cout << "Multiplier: " << multi << endl;
    cout << "Penalized edges count: " << count_edges << endl;
}

template <typename Graph>
void del_penalization_rect(Graph &g, Graph &g_pen, float min_lat, float max_lat, float min_long, float max_long){
    if(boost::num_vertices(g_pen) == 0)
        return;
}
#endif //MAIN_ROUTESPENALIZER_H