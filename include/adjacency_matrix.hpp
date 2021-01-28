#include <string>
#include <tuple>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <queue>
#include <cstdlib>
#include <cstdlib>
#include <ctime>

using City = std::tuple<int, std::string, double, double>;
using Link = std::tuple<int, int, double>;
using Path = std::pair<std::vector<int>, double>;

class AdjacencyMatrix
{
    size_t size_;
    std::vector<std::vector<int>> neighbors_;
    double *arr_;

    void dists(int v)
    {
        std::vector<double> min_distance(size_, std::numeric_limits<double>::infinity());
        std::vector<int> previous(size_, -1);
        min_distance[v] = 0;
        auto cmp = [](Link left, Link right) { return std::get<2>(left) > std::get<2>(right); };
        std::priority_queue<Link, std::vector<Link>, decltype(cmp)> queue(cmp);
        queue.push({v, v, 0});
        while (!queue.empty())
        {
            int where = std::get<0>(queue.top());
            double d = std::get<2>(queue.top());
            queue.pop();
            if (min_distance[where] < d)
                continue;
            for (auto edge : neighbors_[where])
            {
                if (min_distance[edge] > min_distance[where] + arr_[where * size_ + edge])
                {
                    min_distance[edge] = min_distance[where] + arr_[where * size_ + edge];
                    previous[edge] = where;
                    queue.push({edge, where, min_distance[edge]});
                }
            }
        }
        for (auto i = 0; i < size_; i++)
        {
            std::vector<int> path;
            int curr = i;
            path.push_back(curr);
            while (curr != v)
            {
                curr = previous[curr];
                path.push_back(curr);
            }
            paths.emplace_back(path, min_distance[i]);
        }
    }

public:
    std::vector<Path> paths;

    AdjacencyMatrix(size_t size) : size_(size), neighbors_(size)
    {
        arr_ = new double[size * size];
        std::fill_n(arr_, size * size, std::numeric_limits<double>::infinity());
        for (auto i = 0; i < size; i++)
        {
            arr_[i * size + i] = 0.f;
        }
    };
    ~AdjacencyMatrix()
    {
        delete[] arr_;
    };
    void putEdge(int v1, int v2, double weight, bool omni = true)
    {
        if (v1 < 0 || v1 >= size_ || v2 < 0 || v2 >= size_)
            throw std::out_of_range("Out of matrix!");
        if (arr_[v1 * size_ + v2] == std::numeric_limits<double>::infinity())
            neighbors_[v1].push_back(v2);
        arr_[v1 * size_ + v2] = weight;
        if (omni)
        {
            if (arr_[v2 * size_ + v1] == std::numeric_limits<double>::infinity())
                neighbors_[v2].push_back(v1);
            arr_[v2 * size_ + v1] = weight;
        }
    };
    std::vector<int> getNeighbors(int v)
    {
        if (v < 0 || v >= size_)
            throw std::out_of_range("Out of matrix!");
        return neighbors_[v];
    };
    double getEdge(int v1, int v2, double weight, bool omni = true)
    {
        if (v1 < 0 || v1 >= size_ || v2 < 0 || v2 >= size_)
            throw std::out_of_range("Out of matrix!");
        return arr_[v1 * size_ + v2];
    };

    void distsAll()
    {
        for (auto i = 0; i < size_; i++)
        {
            dists(i);
        }
    }

    Path generatePath()
    {
        std::vector<bool> visited(size_);
        Path path;

        int len = 0;

        int last = rand() % size_;
        path.first.push_back(last);

        visited[last] = true;

        int maxlen = rand() % 1000000;
        // std::cout<<"\nmaxlen:\t"<<maxlen<<" ";
        while (path.first.size() < 15 && path.second < maxlen)
        {
            std::vector<int> possible;
            for (auto v : neighbors_[last])
            {
                if (!visited[v])
                    possible.push_back(v);
            }
            if (possible.size() == 0)
                break;
            int next = possible[rand() % possible.size()];
            path.first.push_back(next);
            path.second += arr_[last * size_ + next];
            visited[next] = true;
            last = next;
        }
        std::cout<<path.second<<"\t";
        return path;
    }

    void addPaths(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            paths.push_back(generatePath());
            // std::cout<<i<<"\t"<<paths.back().second<<"\t"<<paths.back().first.size()<<"\n";
        }
    }
};