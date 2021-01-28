#include <iostream>
#include <tuple>
#include <vector>
#include <map>
#include <string>
#include <limits>

#include <cmath>

#include <pugixml.hpp>

#include <typeinfo>

#include "adjacency_matrix.hpp"
#include "dynet_module.hpp"

#include <stdlib.h>
#include <time.h>

constexpr double PI = 3.14159265358979323846;
constexpr double RADIANS = PI / 180.0;
constexpr double RADIUS = 6372797.56085;

double calcDist(double x1, double y1, double x2, double y2)
{
    double dist;

    dist = sin(y1 * RADIANS) * sin(y2 * RADIANS) + cos(y1 * RADIANS) * cos(y2 * RADIANS) * cos((x1 - x2) * RADIANS);
    dist = acos(dist);

    return dist * RADIUS;
}

int main(int argc, char **argv)
{

    srand(time(NULL));

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("data/germany50_network.xml");

    std::vector<City> cities;

    int cityNum = 0;
    for (pugi::xml_node node : doc.child("network").child("networkStructure").child("nodes").children("node"))
    {
        cities.emplace_back(
            cityNum++,
            node.attribute("id").value(),
            std::stod(node.child("coordinates").child("x").child_value()),
            std::stod(node.child("coordinates").child("y").child_value()));
    }
    std::map<std::string, int> citiesMap;
    for (auto &v : cities)
    {
        citiesMap.emplace(std::get<1>(v), std::get<0>(v));
    }

    AdjacencyMatrix mat(cities.size());

    for (pugi::xml_node link : doc.child("network").child("networkStructure").child("links").children("link"))
    {

        std::string sCityName = link.child("source").child_value();
        std::string tCityName = link.child("target").child_value();

        int sCityIndex = citiesMap[sCityName];
        int tCityIndex = citiesMap[tCityName];

        auto &sCity = cities[sCityIndex];
        auto &tCity = cities[tCityIndex];

        double distance = calcDist(std::get<2>(sCity), std::get<3>(sCity), std::get<2>(tCity), std::get<3>(tCity));

        mat.putEdge(sCityIndex, tCityIndex, distance);
    }

    mat.distsAll();

    mat.addPaths(97500);

    auto paths = mat.paths;

    // std::sort(begin(paths), end(paths), [](auto const &t1, auto const &t2) {
    //     return std::get<1>(t1) < std::get<1>(t2);
    // });

    std::sort(begin(paths), end(paths), [](auto const &t1, auto const &t2) {
        return std::get<0>(t1).size() < std::get<0>(t2).size();
    });

    DynetModule dm;

    dynet::initialize(argc, argv);

    dm.run(paths);

    return 0;
}