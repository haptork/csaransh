/*!
 * @file
 * Function to print the results as json
 * */
#ifndef PRINTJSON_CSARANSH_HPP
#define PRINTJSON_CSARANSH_HPP

#include <array>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include <helper.hpp>
#include <results.hpp>

namespace csaransh {
void printJson(std::ofstream &outfile, const std::string &s, const Info &i, int id, const int &nDefects, const int &nClusters, 
               const int &maxClusterSizeI, const int &maxClusterSizeV, const double &inClusterFractionI, 
               const double &inClusterFractionV, const DefectVecT &defects, 
               const std::array<std::vector<double>, 2> &dists, const std::array<std::vector<double>, 2> &angles, 
               const std::unordered_map<int, std::vector<int>> clusters, const std::unordered_map<int, featT> &feats);
}

#endif