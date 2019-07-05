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

void addRectToList(rectangle rect, list<rectangle> &rect_list){
    int new_rect_id = 0;
    for(rectangle &elem: rect_list){
        if(elem.id > new_rect_id)
            new_rect_id = elem.id;
    }
    rect.id = new_rect_id + 1;
    rect_list.push_back(rect);
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

bool delElemList(rectangle rect, list<rectangle> &rect_list){
    auto itrect = rect_list.begin();
    while(itrect != rect_list.end()){
        if(rect.min_lat == itrect->min_lat
            && rect.max_lat == itrect->max_lat
            && rect.min_long == itrect->min_long
            && rect.max_long == itrect->max_long){
            rect_list.erase(itrect);
            return true;
        }
        ++itrect;
    }
    return false;
}

rectangle findRectInList(rectangle rect, list<rectangle> &rect_list){
    for(rectangle &elem: rect_list){
        if(rect.min_lat == elem.min_lat
            && rect.max_lat == elem.max_lat
            && rect.min_long == elem.min_long
            && rect.max_long == elem.max_long)
            return elem;
    }
    return fillRect(0.0, 0.0, 0.0, 0.0, 0);
}

json11::Json getRects(list<rectangle> &rect_list){
    vector<json11::Json> jsonRects;
    auto itrect = rect_list.begin();
    while(itrect != rect_list.end()){
        cout << itrect->toString() << endl;
        auto rect = json11::Json::array {
                itrect->id, itrect->multi, itrect->min_lat, itrect->min_long, itrect->max_lat, itrect->max_long
        };
        jsonRects.push_back(rect);
        ++itrect;
    }
    return jsonRects;
}

