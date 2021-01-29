#include "logisticRegression.hpp"

LogisticRegressionModule::LogisticRegressionModule(double step,std::string path){
    this->step = step;
    this->path = path;
    initialize();
}
LogisticRegressionModule::~LogisticRegressionModule(){
    file.close();
}

void LogisticRegressionModule::initialize()
{
    file.open(path, std::ios::in | std::ios::app);
    file<<"\n";
    for (int i = 0; i < NUMBER_OF_INPUTS; i++)
    {
        weights.push_back(0.0);
    }
}

double LogisticRegressionModule::sigmoid(std::vector<double> path)
{
    return 1.0 / (1 + std::exp((-1) * logisticFunction(path)));
}

double LogisticRegressionModule::logisticFunction(std::vector<double> path)
{
    double result = 0.0;
    for (int i = 0; i < path.size(); i++)
    {
        result += path[i] * weights[i];
    }
    return result;
}

double LogisticRegressionModule::predict(std::vector<double> path)
{
    return sigmoid(path);
}

double LogisticRegressionModule::cost(double prediction, double classification){
    
    double cost = (-1)*(classification*log(prediction) + (1.0 -classification)*log(1-prediction));
    file<<cost<<"\t"<< (decision(prediction) == classification)<<"\n";
    return cost;
}

void LogisticRegressionModule::train(std::vector<double> path, double classification){
    double prediction = predict(path);
    double error = cost(prediction, classification);
    updateWeights(path, prediction, classification);
    step *= 0.999;
}

void LogisticRegressionModule::updateWeights(std::vector<double> path, double prediction, double classification){
    for(int i=0; i< weights.size(); i++){
        weights[i] = weights[i] - step*(prediction-classification)*path[i];
    }
}

double LogisticRegressionModule::decision(double prediction){
    return prediction >= DECISION_BOUNDARY ? 1.0 : 0.0;
}

double LogisticRegressionModule::getPrediction(std::vector<double> path){
    return decision(predict(path));
}