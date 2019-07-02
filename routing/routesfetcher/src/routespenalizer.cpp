#include "routespenalizer.h"

// Fill rect
rectangle fillRect(float min_lat, float max_lat, float min_long, float max_long, int multi){
    rectangle tmp{};
    tmp.min_lat = min_lat;
    tmp.max_lat = max_lat;
    tmp.min_long = min_long;
    tmp.max_long = max_long;
    tmp.multi = multi;

    return tmp;
}

// Check POST request input values
bool checkInput(rectangle rect){
    if(rect.min_lat == 0.0 || rect.max_lat == 0.0 || rect.min_long == 0.0 || rect.max_long == 0.0)
        return false;
    if(rect.min_lat > rect.max_lat)
        return false;
    if(rect.min_long > rect.max_long)
        return false;
    return true;
}

// Check if a coordinate is in rect
bool vertexInRect(rectangle rect, float lat, float lon){
    if(lat < rect.min_lat || lat > rect.max_lat)
        return false;
    if(lon < rect.min_long || lon > rect.max_long)
        return false;
    return true;
}

