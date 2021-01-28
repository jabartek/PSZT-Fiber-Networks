#pragma once
#include "xor.hpp"
#include "adjacency_matrix.hpp"

class DynetModule
{

public:
    void run(std::vector<Path>& paths)
    {
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
    }
};