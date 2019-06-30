#include <cpprest/filestream.h>
#include <boost/program_options.hpp>
#include <boost/graph/graph_utility.hpp>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>
#include <cpprest/ws_client.h>
#include <cpprest/containerstream.h>
#include <cpprest/interopstream.h>
#include <cpprest/rawptrstream.h>
#include <cpprest/producerconsumerstream.h>
#include <chrono>
#include <KDTree.h>
#include <Point.h>
#include <json11.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <src/routesfetcher.h>
#include <src/routespenalizer.h>
#include <boost/archive/text_oarchive.hpp>
#include <iostream>
#include <sstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/graph/copy.hpp>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;
using namespace web::http::experimental::listener;
using namespace web::experimental::web_sockets::client;
using namespace arlib;
using namespace std;


using Graph = boost::adjacency_list<boost::vecS, 
									boost::vecS,
									boost::bidirectionalS, 
									Location,
									boost::property<boost::edge_weight_t, float>>;
using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
using Edge = typename boost::graph_traits<Graph>::edge_descriptor;

class RoutesDealer
{
public:
    RoutesDealer() {}
    RoutesDealer(utility::string_t url, Graph &g, KDTree<2, int> &tree);

    pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

private:
    void handle_get(http_request request);
    void handle_post(http_request request);
    
    Graph g, g_pen;
    bool is_pen;
    KDTree<2, int> tree;
    http_listener m_listener;
};


RoutesDealer::RoutesDealer(utility::string_t url, Graph &g, KDTree<2, int> &tree) : m_listener(url)
{
    m_listener.support(methods::GET, std::bind(&RoutesDealer::handle_get, this, std::placeholders::_1));
    m_listener.support(methods::POST, std::bind(&RoutesDealer::handle_post, this, std::placeholders::_1));
    
    this->g = g;
    this->tree = tree;
    this->is_pen = 0;
}


void send_error(http_request message, std::string body) {
    http_response response(status_codes::BadRequest);
    response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    response.set_body(body);
    message.reply(response);
}

void RoutesDealer::handle_post(http_request request)
{
	cout << "\nHandle POST" << endl;
    
    try {
		int perc = 0;
		auto body = request.extract_string().get();
        cout << "body: " << body << endl;
		//auto myMap = mappify(body);
		map<utility::string_t, utility::string_t> myMap = uri::split_query(body);
		for(auto const& p: myMap)
        	std::cout << '{' << p.first << " => " << p.second << '}' << '\n';
        
		if (myMap.find("change") != myMap.end()) {
            is_pen = boost::lexical_cast<bool>(myMap["change"]);
		}
        
		if (myMap.find("perc") != myMap.end()) {
         
			perc = stoi(myMap["perc"]);
        } else send_error(request, "Percentage not assigned.");
		
		if(perc > 0)
			is_pen = 1;
		
		cout << "is_pen: " << is_pen << endl;
		
		//boost::print_graph(g);
		//boost::print_vertices(g);

		
		bool done = penalize_edges(g, perc);
		
		cout << "done: " << done << endl;
		
		http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.set_body("resresres");
        request.reply(response);
        cout << endl;
    }
    catch (const std::invalid_argument& e) {
		cout << "Error Argument" << endl;
        send_error(request, e.what());
    }
    catch (...) {
		cout << "Error" << endl;
        send_error(request, "ERROR!");
    }
}

void RoutesDealer::handle_get(http_request request)
{
	cout << "\nHandle GET" << endl;
	
    try {
        bool reroute = false;
        float lat = 0.0, lon = 0.0;
        auto url_message= uri::decode(request.relative_uri().to_string());
		
		cout << url_message << endl;
		
        url_message.erase(0,2);
        map<utility::string_t, utility::string_t> keyMap = uri::split_query(url_message);
        int num_routes;
        if (keyMap.find("s_lat") != keyMap.end()) {
            lat = stof(keyMap["s_lat"]);
        } else send_error(request, "Starting latitude missing");
        if (keyMap.find("s_lon") != keyMap.end()) {
            lon = stof(keyMap["s_lon"]);
        } else send_error(request, "Starting longitude missing");
        Vertex start;
        auto start_get_vertices = chrono::steady_clock::now();
        get_vertex(lat, lon, tree, start);
        if (keyMap.find("e_lat") != keyMap.end()) {
            lat = stof(keyMap["e_lat"]);
        } else send_error(request, "Destination latitude missing");
        if (keyMap.find("e_lon") != keyMap.end()) {
            lon = stof(keyMap["e_lon"]);
        } else send_error(request, "Destination longitude missing");
        if (keyMap.find("n_routes") != keyMap.end()) {
            num_routes = std::stoi(keyMap["n_routes"]);
        }
        else num_routes = 1;
        Vertex end;
        get_vertex(lat, lon, tree, end);
        auto end_get_vertices = chrono::steady_clock::now();
        logElapsedMillis("Found vertices in graph", start_get_vertices, end_get_vertices);
        if (keyMap.find("reroute") != keyMap.end()) {
            reroute = parseBoolean(keyMap["reroute"]);
        }
        
        Graph g_tmp;
        if(is_pen)
            copy_graph(g_pen, g_tmp);
        else
            copy_graph(g, g_tmp);
        
        auto paths = get_alternative_routes(g_tmp, start, end, num_routes, 0.9, reroute);
        
        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.set_body(paths.dump());
        request.reply(response);
        cout << endl;
    }
    catch (const std::invalid_argument& e) {
        send_error(request, e.what());
    }
    catch (...) {
        send_error(request, "ERROR!");
    }
}


int main(int argc, char* argv[]) {
    namespace po = boost::program_options;
    using dataset = vector<pair<Point<2>, int>>;
    po::options_description desc("Allowed options");
    std::string endpoint;
    desc.add_options()
            ("endpoint", po::value<string>()->default_value("http://0.0.0.0:1337/"), "Insert the server endpoint")
            ;
    po::variables_map opts;
    po::positional_options_description p;
    p.add("endpoint", -1);
    po::store(po::command_line_parser(argc,argv).options(desc).positional(p).run(), opts);
    try {
        po::notify(opts);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    endpoint = opts["endpoint"].as<std::string>();
    Graph g;
    auto start = chrono::steady_clock::now();
    dataset data = location_graph_from_string("weights", "ids", g, true);
    auto end = chrono::steady_clock::now();
    logElapsedMillis("Loaded coordinates and weights", start, end);
    KDTree<2, int> kd(data);
    utility::string_t address = U(endpoint);
    RoutesDealer listener(address, g, kd);
    listener.open().wait();

    cout << utility::string_t(U("Listening for requests at 0.0.0.0:1337"))  << endl;

    std::string cline;
    std::wcout << U("Hit Enter to close the listener...") << endl;
    std::getline(std::cin, cline);

    listener.close().wait();
}

