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

    // Assignment operator.
    bool operator ==(const rectangle& st)
    {
        return min_lat == st.min_lat && max_lat == st.max_lat && min_long == st.min_long && min_long == st.max_lat;
    }
};
// Rect List


rectangle fillRect(float min_lat, float max_lat, float min_long, float max_long, int multi);
bool checkInput(rectangle rect);
bool vertexInRect(rectangle rect, float lat, float lon);
bool delElemList(rectangle rect, list<rectangle> &rect_list);
json11::Json getRects(list<rectangle> &rect_list);

template <typename Graph>
json11::Json add_penalization_rect(Graph &g, Graph &g_pen, rectangle rect){
    cout << "Add Penalization rect" << endl;
    typedef typename boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    typename Graph::vertex_iterator v, vend;

    if(boost::num_vertices(g_pen) == 0)
        copy_graph(g, g_pen);

    vector<json11::Json> penalized_path;

    int count_edges = 0;
    IndexMap index = get(boost::vertex_index, g_pen);
    for (boost::tie(ei, ei_end) = edges(g_pen); ei != ei_end; ++ei) {
        std::pair<Edge, bool> ed = boost::edge(index[source(*ei, g_pen)], index[target(*ei, g_pen)], g_pen);

        float source_lat = g_pen[source(*ei, g_pen)].lat;
        float target_lat = g_pen[target(*ei, g_pen)].lat;

        float source_lon = g_pen[source(*ei, g_pen)].lon;
        float target_lon = g_pen[target(*ei, g_pen)].lon;

        if(vertexInRect(rect, source_lat, source_lon) || vertexInRect(rect, target_lat, target_lon)){
            //float weight = get(boost::edge_weight_t(), g_pen, ed.first);
            //cout << "source_lat: " << source_lat << " source_lon: " << source_lon;
            //cout << " target_lat: " << target_lat << " target_lon: " << target_lon;
            //cout << " Weight: " << weight;
            get(boost::edge_weight_t(), g_pen, ed.first) *= rect.multi;
            //float new_weight = get(boost::edge_weight_t(), g_pen, ed.first);
            //cout << ", new Weight: " << new_weight << endl;

            //fill json
            vector<json11::Json::array> json_arr;
            auto start = json11::Json::array {source_lat, source_lon,0};
            auto end = json11::Json::array {
                target_lat, target_lon,get(boost::edge_weight_t(), g_pen, ed.first)
            };

            json_arr.push_back(start);
            json_arr.push_back(end);
            penalized_path.push_back(json_arr);

            count_edges++;
        }
    }
    cout << "Multiplier: " << rect.multi << endl;
    cout << "Penalized edges count: " << count_edges << endl;

    return penalized_path;
}

template <typename Graph>
void del_penalization_rect(Graph &g, Graph &g_pen, rectangle rect){
    cout << "Add Penalization rect" << endl;
    typedef typename boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    typename Graph::vertex_iterator v, vend;

    if(boost::num_vertices(g_pen) == 0)
        return;

    int count_edges = 0;
    IndexMap index = get(boost::vertex_index, g_pen);
    for (boost::tie(ei, ei_end) = edges(g_pen); ei != ei_end; ++ei) {
        std::pair<Edge, bool> ed = boost::edge(index[source(*ei, g_pen)], index[target(*ei, g_pen)], g_pen);

        float source_lat = g_pen[source(*ei, g_pen)].lat;
        float target_lat = g_pen[target(*ei, g_pen)].lat;

        float source_lon = g_pen[source(*ei, g_pen)].lon;
        float target_lon = g_pen[target(*ei, g_pen)].lon;

        if(vertexInRect(rect, source_lat, source_lon) || vertexInRect(rect, target_lat, target_lon)){
            get(boost::edge_weight_t(), g_pen, ed.first) /= rect.multi;
            count_edges++;
        }
    }
    cout << "Multiplier: " << rect.multi << endl;
    cout << "De-Penalized edges count: " << count_edges << endl;
}
#endif //MAIN_ROUTESPENALIZER_H