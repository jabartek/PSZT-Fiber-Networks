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
#include "xor.hpp"

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


    dynet::initialize(argc, argv);

    const unsigned ITERATIONS = 250;

    ParameterCollection m;
    SimpleSGDTrainer trainer(m);

    const unsigned HS1 = 5;
    const unsigned HS2 = 5;

    Parameter p_W1 = m.add_parameters({HS1, 50});
    Parameter p_b1 = m.add_parameters({HS1});
    Parameter p_W2 = m.add_parameters({HS2, HS1});
    Parameter p_b2 = m.add_parameters({HS2});

    Parameter p_V = m.add_parameters({1, HS2});
    Parameter p_a = m.add_parameters({1});

    // Static declaration of the computation graph.
    ComputationGraph cg;
    Expression W1 = parameter(cg, p_W1);
    Expression b1 = parameter(cg, p_b1);
    Expression W2 = parameter(cg, p_W2);
    Expression b2 = parameter(cg, p_b2);
    Expression V = parameter(cg, p_V);
    Expression a = parameter(cg, p_a);

    // Set x_values to change the inputs to the network.
    vector<dynet::real> x_values(50);
    Expression x = input(cg, {50}, &x_values);
    dynet::real y_value; // Set y_value to change the target output.
    Expression y = input(cg, &y_value);

    Expression h1 = tanh(W1 * x + b1);
    Expression h2 = W2 * h1 + b2;
    Expression y_pred = rectify(V * h2 + a);
    Expression loss_expr = squared_distance(y_pred, y);

    // Train the parameters.
    double loss = 0;
    unsigned iter = 0;
    double acc = 0;
    double lastacc = 0;
    while (true)
    {
        loss = 0;
        lastacc = acc;
        acc = 0;

        for (unsigned mi = 0; mi < 200; ++mi)
        {
            int pathNo = rand() % 100000;
            if (std::get<0>(paths[pathNo]).size() == 1)
            {
                --mi;
                continue;
            }
            for (int j = 0; j < 50; j++)
            {
                x_values[j] = 0;
            }
            for (auto v : std::get<0>(paths[pathNo]))
            {
                x_values[v] = 1;
            }
            y_value = (std::get<1>(paths[pathNo])) > 600000 ? 25 : (std::get<1>(paths[pathNo]) > 400000 ? 20 : (std::get<1>(paths[pathNo]) > 200000 ? 15 : 10));
            // std::cout << "ex:\t" << y_value << "\tpred:\t" << dynet::as_scalar(y_pred.value()) << "\n";
            loss += (as_scalar(cg.forward(loss_expr)));
            if (fabs(y_value - dynet::as_scalar(y_pred.value())) < 2.5f)
                acc++;
            cg.backward(loss_expr);
            trainer.update();
        }
        loss /= 200;
        acc /= 200;
        cout << iter++ << "\t" << acc << endl;
        if (acc > 0.99 || iter > 1000)
            break;
    }

    const unsigned TESTS = 20;

    for (unsigned i = 0; i < TESTS; ++i)
    {
        int pathNo = rand() % 100000;
        for (int j = 0; j < 50; j++)
        {
            x_values[j] = 0;
        }
        for (auto v : std::get<0>(paths[pathNo]))
        {
            x_values[v] = 1;
        }
        y_value = (std::get<1>(paths[pathNo])) > 600000 ? 25 : (std::get<1>(paths[pathNo]) > 400000 ? 20 : (std::get<1>(paths[pathNo]) > 200000 ? 15 : 10));
        cg.forward(y_pred);
        std::cout << "Exp:\t" << y_value << "\tRnd:\t" << std::round(as_scalar(y_pred.value())) << "\tPred:\t" << as_scalar(y_pred.value()) << "\tDiff:\t" << abs(y_value - (as_scalar(y_pred.value()))) << "\n";
    }

    // // Check whether our ComputationGraph learns correctly or not.
    // x_values[0] = -1;   // Set input value
    // x_values[1] = -1;   // Set input value
    // cg.forward(y_pred); // Calculate until y_pred node
    // std::cout << "[-1,-1] -1 : " << as_scalar(y_pred.value()) << std::endl;
    // x_values[0] = -1;
    // x_values[1] = 1;
    // cg.forward(y_pred);
    // std::cout << "[-1, 1]  1 : " << as_scalar(y_pred.value()) << std::endl;
    // x_values[0] = 1;
    // x_values[1] = -1;
    // cg.forward(y_pred);
    // std::cout << "[ 1,-1]  1 : " << as_scalar(y_pred.value()) << std::endl;
    // x_values[0] = 1;
    // x_values[1] = 1;
    // cg.forward(y_pred);
    // std::cout << "[ 1, 1] -1 : " << as_scalar(y_pred.value()) << std::endl;

    // Output the model and parameter objects to a file.
    TextFileSaver saver("xor.model");
    saver.save(m);
    return 0;
}