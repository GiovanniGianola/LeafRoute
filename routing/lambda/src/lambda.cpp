#include <json11.hpp>
#include <src/routesfetcher.h>
#include <src/routespenalizer.h>
#include <src/utils.hpp>
#include <aws/lambda-runtime/runtime.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/copy.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <cpprest/uri.h>

using namespace arlib;
using namespace aws::lambda_runtime;
using namespace json11;
using namespace std;


using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
        boost::bidirectionalS, Location,
        boost::property<boost::edge_weight_t, float>>;
using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
using Edge = typename boost::graph_traits<Graph>::edge_descriptor;

Graph g, g_pen;
bool is_pen;
string func[2] = {"add", "del"};
list<rectangle> rect_list;
 /*
  * https://docs.aws.amazon.com/apigateway/latest/developerguide/set-up-lambda-proxy-integrations.html#api-gateway-simple-proxy-for-lambda-output-format
  * API Gateway Input format
  * {
     "resource": "Resource path",
     "path": "Path parameter",
     "httpMethod": "Incoming request's method name"
     "headers": {String containing incoming request headers}
     "multiValueHeaders": {List of strings containing incoming request headers}
     "queryStringParameters": {query string parameters }
     "multiValueQueryStringParameters": {List of query string parameters}
     "pathParameters":  {path parameters}
     "stageVariables": {Applicable stage variables}
     "requestContext": {Request context, including authorizer-returned key-value pairs}
     "body": "A JSON string of the request payload."
     "isBase64Encoded": "A boolean flag to indicate if the applicable request payload is Base64-encode"

  * }
  * Only "queryStringParameters" will be used here
  *
  * API Gateway Output format
  * {
    "isBase64Encoded": true|false,
    "statusCode": httpStatusCode,
    "headers": { "headerName": "headerValue", ... },
    "body": "..."
  * }
  */
invocation_response my_handler(invocation_request const& request) {
    auto start_read_files = chrono::steady_clock::now();
    if (boost::num_vertices(g) == 0) {
        std::ifstream in("data.btl");{
            boost::archive::text_iarchive ia(in);
            ia >> g;
        }
    }

    auto end_read_files = chrono::steady_clock::now();
    logElapsedMillis("Loaded coordinates and weights", start_read_files, end_read_files);
    string err;
    auto js_obj = json11::Json::parse(request.payload, err);
    auto parameters = js_obj["queryStringParameters"];
    auto headers = Json::object {{"Access-Control-Allow-Origin", "*"}};

    cout << "httpMethod: " << js_obj["httpMethod"].dump() << endl;
    cout << "path: " << js_obj["path"].dump() << " resource: " << js_obj["resource"].dump() << endl;
    cout << "parameters: " << parameters.dump() << endl;

    if(js_obj["httpMethod"].dump() == "\"GET\""){
        cout << "\nHandle GET" << endl;
        if(js_obj["path"].dump() == "\"/getroutes\""){
            float s_lat = 0.0, s_lon = 0.0, e_lat = 0.0, e_lon = 0.0;
            bool reroute;
            int num_routes = 1;
            try {
                s_lat = stof(parameters["s_lat"].string_value());
                s_lon = stof(parameters["s_lon"].string_value());
                e_lat = stof(parameters["e_lat"].string_value());
                e_lon = stof(parameters["e_lon"].string_value());
            }
            catch (invalid_argument &) {
                return invocation_response::failure("Start and end coordinates must be provided", "InvalidRequest");
            }
            reroute = parseBoolean(parameters["reroute"].string_value());
            try {
                auto num_routes_str = parameters["n_routes"].string_value();
                num_routes = stoi(num_routes_str);
            }
            catch (invalid_argument &) {
                num_routes = 1;
            }
            Vertex start;
            auto start_get_vertices = chrono::steady_clock::now();
            get_vertex(s_lat, s_lon, g, start);
            Vertex end;
            get_vertex(e_lat, e_lon, g, end);
            auto end_get_vertices = chrono::steady_clock::now();
            logElapsedMillis("Found vertices", start_get_vertices, end_get_vertices);

            cout << "s_lat: " << s_lat << endl;
            cout << "s_lon: " << s_lon << endl;
            cout << "e_lat: " << e_lat << endl;
            cout << "e_lon: " << e_lon << endl;
            cout << "num_routes: " << num_routes << endl;
            cout << "reroute: " << reroute << endl;

            auto paths = get_alternative_routes(g, start, end, num_routes, 0.9, reroute);
            cout << "PathFound" << endl;
            Json response = Json::object{
                    {"isBase64Encoded", false},
                    {"statusCode", 200},
                    {"headers", headers},
                    {"body", paths.dump()}
            };
            cout << response.dump() << std::endl;
            return invocation_response::success(response.dump(), "application/json");
        }else
            return invocation_response::failure("Invalid GET method", "InvalidRequest");
    }else
        return invocation_response::failure("Invalid HTTP method", "InvalidRequest");
}


int main() {
    run_handler(my_handler);
    return 0;
}
