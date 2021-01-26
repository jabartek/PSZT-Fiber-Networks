#include <iostream>
#include <tuple>
#include <vector>
#include <map>
#include <string>
#include <limits>
#include <algorithm>
#include <random>
#include <cmath>

#include <pugixml.hpp>

#include <typeinfo>

#include "adjacency_matrix.hpp"
#include "logisticRegression.hpp"

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

double getClass(double distance)
{
    distance /= 1000;
    if (distance < 200.0)
        return 3.0;
    else if (distance >= 200.0 && distance < 400.0)
        return 2.0;
    else if (distance >= 400.0 && distance < 600.0)
        return 1.0;
    else
        return 0.0;
}

double getModuleClass(double result1, double result2)
{
    if (result1 == 0.0)
    {
        if (result2 == 0.0)
            return 0.0;
        else
            return 1.0;
    }
    else
    {
        if (result2 == 0.0)
            return 2.0;
        else
            return 3.0;
    }
}

std::vector<double> getFormattedInput(std::vector<int> path)
{
    std::vector<double> inputs(51);
    fill(inputs.begin(), inputs.end(), 0.0);
    for (auto city : path)
    {
        inputs[city] = 1.0;
    }
    inputs[inputs.size() - 1] = 1.0;
    return inputs;
}

std::pair<std::vector<Path>, std::vector<Path>> getTrainAndValidationDataset(std::vector<Path>& data, double trainPercentage =0.8){
    auto rng = std::default_random_engine{};
    shuffle(data.begin(), data.end(), rng);
    std::vector<Path> train(data.begin(), data.begin()+ data.size()*trainPercentage);
    std::vector<Path> validate(data.begin()+ data.size()*trainPercentage, data.end());
    return std::make_pair(train,validate);
}

int main()
{
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
    mat.addPaths(8500);

    auto paths = mat.paths;
    std::pair<std::vector<Path>, std::vector<Path>> data = getTrainAndValidationDataset(paths);
    std::vector<Path> train = data.first;
    std::vector<Path> validate = data.second;

    const double step = 3.0;
    LogisticRegressionModule logModuleParityBit(step, "./data/ParityLogistic.csv");
    LogisticRegressionModule logModuleLowerHalf(step, "./data/LowerHalfLogistic.csv");
    LogisticRegressionModule logModuleUpperHalf(step, "./data/UpperHalfLogistic.csv");
    std::vector<double> inputs;

    for (Path path : train)
    {
        // std::cout << "********************************************************" << '\n';
        double classification = getClass(path.second);
        if (classification == 0 || classification == 1)
        {
            logModuleParityBit.train(getFormattedInput(path.first), 0.0);
            if (classification == 0)
            {
                logModuleLowerHalf.train(getFormattedInput(path.first), 0.0);
            }
            else
            {
                logModuleLowerHalf.train(getFormattedInput(path.first), 1.0);
            }
        }
        else if (classification == 2 || classification == 3)
        {
            logModuleParityBit.train(getFormattedInput(path.first), 1.0);
            if (classification == 2)
            {
                logModuleUpperHalf.train(getFormattedInput(path.first), 0.0);
            }
            else
            {
                logModuleUpperHalf.train(getFormattedInput(path.first), 1.0);
            }
        }
    }

    double badGuesses = 0.0;
    double goodGuesses = 0.0;
    for (Path path : validate)
    {
        double predict2;
        double predict1 = logModuleParityBit.getPrediction(getFormattedInput(path.first));
        if (predict1 == 0.0)
            predict2 = logModuleLowerHalf.getPrediction(getFormattedInput(path.first));
        else
            predict2 = logModuleUpperHalf.getPrediction(getFormattedInput(path.first));

        if (getModuleClass(predict1, predict2) == getClass(path.second))
            goodGuesses++;
        else
            badGuesses++;
    }

    std::cout<<"STATS:\n";
    std::cout<<"GOOD GUESSES:\t"<<goodGuesses<<"\n";
    std::cout<<"BAD GUESSES:\t"<<badGuesses<<"\n";
    std::cout<<"% GOOD:\t"<<(goodGuesses/validate.size()) * 100<<"\n";
    std::cout<<"% BAD:\t"<<(badGuesses/validate.size()) * 100<<"\n";

    return 0;
}