#include <iostream>
#include <tuple>
#include <vector>
#include <map>
#include <string>

#include <pugixml.hpp>

using City = std::tuple<int, std::string, double, double>;

int main(){
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("data/germany50_network.xml");

    std::vector<City> cities;

    int cityNum = 0;
    for(pugi::xml_node node : doc.child("network").child("networkStructure").child("nodes").children("node")){
        cities.emplace_back(
            ++cityNum,
            node.attribute("id").value(),
            node.child("coordinates").child("x").child_value(),
            node.child("coordinates").child("y").child_value());
    }
    std::map<std::string, int> citiesMap;
    for(auto &v : cities){
        citiesMap.emplace(std::get<1>(v), std::get<0>(v));
    }
    return 0;
}