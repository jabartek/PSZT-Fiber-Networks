#pragma once
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>

class LogisticRegressionModule{
public:
    LogisticRegressionModule(double step, std::string path);
    ~LogisticRegressionModule();
    void train(std::vector<double> path, double classification);
    double getPrediction(std::vector<double> path);

private:
    double step;
    const int NUMBER_OF_INPUTS = 51;
    const double DECISION_BOUNDARY = 0.5;
    std::string path;
    std::fstream file;

    std::vector<double> weights;
    double predict(std::vector<double> path);
    void initialize();
    double sigmoid(std::vector<double> path);
    double logisticFunction(std::vector<double> path);
    double cost(double prediction, double classification);
    void updateWeights(std::vector<double> path, double prediction, double classification);
    double decision(double prediction);

};