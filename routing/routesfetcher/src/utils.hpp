#ifndef MAIN_UTILS_HPP
#define MAIN_UTILS_HPP

#include <string>
using namespace std;

bool parseBoolean(const string &str);

void logElapsedMillis(
        const std::string reason,
        const std::chrono::time_point<std::chrono::steady_clock> start,
        const std::chrono::time_point<std::chrono::steady_clock> end);

map<std::string, std::string> mappify(std::string const& s);
#endif //MAIN_UTILS_HPP
