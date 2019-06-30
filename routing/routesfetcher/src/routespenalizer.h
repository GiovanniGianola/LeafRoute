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
bool penalize_edges(Graph &g, int perc){
	 cout << "wewe4" << endl;
	 cout << "wewe4d" << endl;
	 return true;
}
#endif //MAIN_ROUTESPENALIZER_H