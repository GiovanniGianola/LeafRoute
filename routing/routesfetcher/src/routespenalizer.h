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
    int id;
    float min_lat;
    float max_lat;
    float min_long;
    float max_long;
    int multi;

    // Assignment operator.
    bool operator ==(const rectangle& st){
        return min_lat == st.min_lat
        && max_lat == st.max_lat
        && min_long == st.min_long
        && min_long == st.max_lat
        && id == st.id;
    }

    // ToString Method
    string toString(){
        std::string ts = "id: " + to_string(id) + ", multi = " + to_string(multi)
                + ", min_lat = " + to_string(min_lat)
                + ", max_lat = " + to_string(max_lat)
                + ", min_long = " + to_string(min_long)
                + ", max_long = " + to_string(max_long);
        return ts;
    }
};
// Rect List


rectangle fillRect(float min_lat, float max_lat, float min_long, float max_long, int multi);
void addRectToList(rectangle rect, list<rectangle> &rect_list);
bool checkInput(rectangle rect);
bool vertexInRect(rectangle rect, float lat, float lon);
rectangle findRectInList(rectangle rect, list<rectangle> &rect_list);
bool delElemList(rectangle rect, list<rectangle> &rect_list);
json11::Json getRects(list<rectangle> &rect_list);

template <typename Graph>
void add_penalization_rect(Graph &g, Graph &g_pen, rectangle rect){
    cout << "Add Penalization rect" << endl;
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

        if(vertexInRect(rect, source_lat, source_lon) || vertexInRect(rect, target_lat, target_lon)){
            //float weight = get(boost::edge_weight_t(), g_pen, ed.first);
            //cout << "source_lat: " << source_lat << " source_lon: " << source_lon;
            //cout << " target_lat: " << target_lat << " target_lon: " << target_lon;
            //cout << " Weight: " << weight;
            get(boost::edge_weight_t(), g_pen, ed.first) *= rect.multi;
            //float new_weight = get(boost::edge_weight_t(), g_pen, ed.first);
            //cout << ", new Weight: " << new_weight << endl;

            count_edges++;
        }
    }
    cout << "Multiplier: " << rect.multi << endl;
    cout << "Penalized edges count: " << count_edges << endl;
}

template <typename Graph>
void del_penalization_rect(Graph &g_pen, rectangle rect){
    cout << "Remove Penalization rect" << endl;
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
    cout << "De-Multiplier: " << rect.multi << endl;
    cout << "De-Penalized edges count: " << count_edges << endl;
}

template <typename Graph>
void export_graph(Graph &g_pen){
    cout << "Exporting graph" << endl;

    /*std::cout << "iterate over vertices, then over its neighbors\n";
    auto vs = boost::vertices(g_pen);
    for (auto vit = vs.first; vit != vs.second; ++vit) {
        auto neighbors = boost::adjacent_vertices(*vit, g_pen);
        for (auto nit = neighbors.first; nit != neighbors.second; ++nit)
            std::cout << *vit << ' ' << *nit << std::endl;
    }*/
    typedef typename boost::property_map<Graph, boost::vertex_index_t>::type IndexMap;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typename boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    typename Graph::vertex_iterator v, vend;

    if(boost::num_vertices(g_pen) == 0)
        return;

    int count_edges = 0;
    int count_vertices = 0;
    std::cout << "iterate directly over edges\n";
    IndexMap index = get(boost::vertex_index, g_pen);
    auto current_node = -1;
    for (boost::tie(ei, ei_end) = edges(g_pen); ei != ei_end; ++ei) {
        std::pair<Edge, bool> ed = boost::edge(index[source(*ei, g_pen)], index[target(*ei, g_pen)], g_pen);

        float source_lat = g_pen[source(*ei, g_pen)].lat;
        float source_lon = g_pen[source(*ei, g_pen)].lon;

        float w = get(boost::edge_weight_t(), g_pen, ed.first);


        cout << source(*ei, g_pen) << ' ' << target(*ei, g_pen) << ' ' << w << ' ';
        count_edges++;
        if(current_node != source(*ei, g_pen)) {
            cout <<" - " << source_lon << ' ' << source_lat << endl;
            count_vertices++;
            current_node = source(*ei, g_pen);
        }else{
            cout << endl;
        }

    }
    cout << "count_edges: " << count_edges << endl;
    cout << "count_vertices: " << count_vertices << endl;
}
#endif //MAIN_ROUTESPENALIZER_H