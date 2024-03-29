#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <cluster2features.hpp>
#include <helper.hpp>

using Coords = csaransh::Coords;

// dot product of two vectors
double dotv(const Coords &a, const Coords &b) {
  auto res = 0.0;
  for (size_t i = 0; i < a.size(); ++i) {
    res += a[i] * b[i];
  }
  return res;
}

// cross product of two vectors
auto crossProd(const Coords &v1, const Coords &v2) {
  Coords prod;
  prod[0] = v1[1] * v2[2] - v1[2] * v2[1];
  prod[1] = v1[2] * v2[0] - v1[0] * v2[2];
  prod[2] = v1[0] * v2[1] - v1[1] * v2[0];
  return prod;
}

// calculates angle between ba and ca
auto calcAngle(Coords a, Coords b, Coords c) {
  Coords v1, v2;
  constexpr double pi = 3.141592653589793;
  for (size_t i = 0; i < a.size(); ++i) {
    v1[i] = b[i] - a[i];
    v2[i] = c[i] - a[i];
  }
  auto res = dotv(v1, v2) / (std::sqrt(dotv(v1, v1) * dotv(v2, v2)));
  return std::acos(res) * 180.0 / pi;
}

// histograms for angles, distances and adjacency for cluster characterization
csaransh::featT csaransh::pairHists(const std::vector<std::array<double, 3>> &v,
                                    const std::vector<bool> &v2,
                                    double latConst) {
  using std::array;
  using std::vector;
  constexpr auto maxDistAssumption = 1.0; //
  constexpr size_t distBins = 40;
  constexpr double distBinSize = maxDistAssumption / distBins;
  array<double, 20> adjacencyHistnn2{};
  array<double, distBins> distHist{};
  std::fill(begin(distHist), end(distHist), 0.0);
  double total = 0;
  double totalAdj = 0;
  auto nn = (std::sqrt(3) * latConst) / 2 + 1e-6;
  auto nn2 = latConst + 1e-6;
  auto nn4 = nn * 2;
  for (size_t i = 0; i < v.size(); ++i) {
    // if (v2[i]) continue;
    std::vector<double> pairDists;
    auto adjacencyCount = 0;
    for (size_t j = 0; j < v.size(); ++j) {
      // if (v2[j]) continue;
      auto dist = calcDist(v[i], v[j]);
      if (dist < nn2) adjacencyCount++;
      if (j > i) {
        if (dist > nn4 * 2) continue; // ?
        pairDists.push_back(dist / latConst);
      }
    }
    if (adjacencyCount >= 20) adjacencyCount = 19;
    adjacencyHistnn2[adjacencyCount]++;
    totalAdj++;
    auto mx = std::max_element(begin(pairDists), end(pairDists));
    for (auto it : pairDists) {
      size_t bin = (it / *mx) / distBinSize;
      if (bin >= distHist.size()) bin = distHist.size() - 1;
      ++distHist[bin];
      ++total;
    }
  }
  if (total > 1e-6)
    for (auto &it : distHist)
      it /= total;
  if (totalAdj > 1e-6) {
    for (auto &it : adjacencyHistnn2)
      it /= totalAdj;
  }
  // angles
  constexpr auto angleBinSize = 5.0;
  constexpr auto maxAngle = 180.0;
  constexpr auto nAngleBins = (size_t)(maxAngle / angleBinSize);
  std::array<double, nAngleBins> angleHist{};
  std::fill(begin(angleHist), end(angleHist), 0);
  total = 0.0;
  for (size_t i = 0; i < v.size(); ++i) {
    if (v2[i]) continue;
    for (size_t j = 0; j < v.size(); ++j) {
      if (i == j || calcDist(v[i], v[j]) > nn4) continue;
      for (size_t k = j + 1; k < v.size(); ++k) {
        if (i == k || v2[j] != v2[k] || calcDist(v[i], v[k]) > nn4) continue;
        auto bin = calcAngle(v[i], v[j], v[k]) / angleBinSize;
        if (bin >= angleHist.size()) bin = angleHist.size() - 1;
        ++angleHist[bin];
        ++total;
      }
    }
  }
  if (total > 1e-6)
    for (auto &it : angleHist)
      it /= total;
  return std::make_tuple(distHist, angleHist, adjacencyHistnn2);
}