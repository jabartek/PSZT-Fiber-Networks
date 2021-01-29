#pragma once
#include "xor.hpp"
#include "adjacency_matrix.hpp"
#include "utils.hpp"

#include <fstream>
#include <sstream>

class DynetModule
{
public:
    DynetModule() : path("./data/NN.csv")
    {
        file.open(path, std::ios::in | std::ios::app);
    }
    ~DynetModule()
    {
        file.close();
    }

private:
    std::string path;
    std::fstream file;

public:
    void run(std::vector<Path> &paths)
    {
        const unsigned ITERATIONS = 250;

        ParameterCollection m;
        AdagradTrainer trainer(m);

        const unsigned HS1 = 24;
        const unsigned HS2 = 16;

        Parameter p_W1 = m.add_parameters({HS1, 50});
        Parameter p_b1 = m.add_parameters({HS1});
        Parameter p_W2 = m.add_parameters({HS2, HS1});
        Parameter p_b2 = m.add_parameters({HS2});

        Parameter p_o2 = m.add_parameters({HS1});

        Parameter p_V = m.add_parameters({1, HS2});
        Parameter p_a = m.add_parameters({1});

        ComputationGraph cg;
        Expression W1 = parameter(cg, p_W1);
        Expression b1 = parameter(cg, p_b1);
        Expression W2 = parameter(cg, p_W2);
        Expression b2 = parameter(cg, p_b2);
        Expression V = parameter(cg, p_V);
        Expression a = parameter(cg, p_a);
        Expression o2 = parameter(cg, p_o2);

        vector<dynet::real> x_values(50);
        Expression x = input(cg, {50}, &x_values);
        dynet::real y_value;
        Expression y = input(cg, &y_value);

        Expression h1 = W1 * x + b1;
        Expression h2 = W2 * cdiv(o2, h1) + b2;
        Expression y_pred = rectify(V * h2 + a);
        Expression loss_expr = squared_distance(y_pred, y);

        double loss = 0;
        unsigned iter = 0;
        double acc = 0;
        double lastacc = 0;
        for (int i = 0; i < 50000; ++i)
        {
            int pathNo = rand() % 100000;
            if (std::get<0>(paths[pathNo]).size() == 1)
            {
                --i;
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
            y_value = getOsnr(std::get<1>(paths[pathNo]));
            cg.forward(y_pred);
            file << i << "\t" << y_value << "\t" << dynet::as_scalar(y_pred.value()) << "\t" << fabs(y_value - dynet::as_scalar(y_pred.value())) << "\n";
            cg.backward(loss_expr);
            trainer.update();
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
            y_value = getOsnr(std::get<1>(paths[pathNo]));
            cg.forward(y_pred);
            std::cout << "Exp:\t" << y_value << "\tRnd:\t" << std::round(as_scalar(y_pred.value())) << "\tPred:\t" << as_scalar(y_pred.value()) << "\tDiff:\t" << abs(y_value - (as_scalar(y_pred.value()))) << "\n";
        }

        // Output the model and parameter objects to a file.
        TextFileSaver saver("xor.model");
        saver.save(m);
    }
};