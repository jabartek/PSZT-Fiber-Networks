#pragma once

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

double getOsnr(double distance)
{
    distance /= 1000;
    if (distance < 200.0)
        return 25.0;
    else if (distance < 400.0)
        return 20.0;
    else if (distance < 600.0)
        return 15.0;
    else
        return 10.0;
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