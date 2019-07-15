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
#include <stdio.h>
#include <stdlib.h>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
#include <boost/archive/text_iarchive.hpp>

#define LAMBDA_PACK "\
    #/bin/bash \n\
    cd .. \n\
    make aws-lambda-package-lambda \n\
    clear\
    "

#define LOAD_GRAPH "\
    #/bin/bash \n\
    zip -m ../lambda/lambda.zip data.btl \n\
    clear\
    "

#define UPLOAD_FUNCTION_AWS "\
    #/bin/bash \n\
    aws lambda update-function-code --function-name leafroute --zip-file fileb://../lambda/lambda.zip \n\
    clear\
    "

#define PIPELINE {LAMBDA_PACK, LOAD_GRAPH, UPLOAD_FUNCTION_AWS}

const string PIPELINE_DESC[] = {"Generating lambda.zip...", "Loading data.btl into lambda.zip...", "Uploading Lambda.zip on AWS..."};


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
            get(boost::edge_weight_t(), g_pen, ed.first) *= rect.multi;
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
bool export_graph(Graph &g_pen){
    // Export Graph in data.btl
    cout << "[START] (1) Exporting Graph in data.btl..." << endl;
    if(boost::num_vertices(g_pen) == 0) {
        cout << "[END] (1) graph not found" << endl;
        return false;
    }
    std::ofstream oss("data.btl");{
        boost::archive::text_oarchive oa(oss);
        oa << g_pen;
    }
    cout << "[END] (1) returned normally" << endl;

    // Generate lambda.zip
    // Load data.btl into lambda.zip
    // Upload Lambda.zip on AWS
    int instr_count = 2;
    for(const auto& op : PIPELINE) {
        cout << "[START] (" << instr_count << ") " << PIPELINE_DESC[instr_count-2] << endl;
        int status_code = system(op);
        if (status_code < 0) {
            std::cout << "[END] (" << instr_count << ") Error: " << strerror(errno) << endl;
            return false;
        }
        else {
            if (WIFEXITED(status_code))
                cout << "[END] (" << instr_count << ") " << "returned normally, exit code " << WEXITSTATUS(status_code) << endl;
            else {
                cout << "[END] (" << instr_count << ") " << "exited abnormally" << endl;
                return false;
            }
        }
        instr_count++;
    }
    return true;
}
#endif //MAIN_ROUTESPENALIZER_H