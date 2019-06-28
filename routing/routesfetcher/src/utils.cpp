#include <string>
#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
using namespace std;

bool parseBoolean(const string &str) {
    return str == "true" || str == "yes" || str == "on";
}

void logElapsedMillis(
        const std::string reason,
        const std::chrono::time_point<std::chrono::steady_clock> start,
        const std::chrono::time_point<std::chrono::steady_clock> end) {
    cout << reason << " in "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms" << endl;
}

map<std::string, std::string> mappify(std::string const& s)
{
	std::map<std::string, std::string> myMap;

	std::string key, val;
	std::istringstream iss(s);

	while(std::getline(std::getline(iss, key, '=') >> std::ws, val))
		myMap[key] = val;

	return myMap;
}