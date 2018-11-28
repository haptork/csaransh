#include <array>
#include <cmath>

#include <dist.hpp>

using Coords = std::array<double, 3>;

double calcDist (Coords a, Coords b) {
    double dist = 0.0;
    for (auto i : {0, 1, 2}) {
      dist += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return std::sqrt(dist);
}
