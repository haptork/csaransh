#include <printJson.hpp>
#include <cluster2features.hpp>
#include <results.hpp>

void printClusterIds(const std::unordered_map<int, std::vector<int>> clusters,
                     std::ofstream &outfile) {
  size_t i = 0;
  for (const auto &it : clusters) {
    outfile << '"' << it.first << "\":";
    outfile << "[";
    csaransh::writeVector(it.second, outfile);
    outfile << "]";
    if (i != clusters.size() - 1) outfile << ", ";
    outfile << "\n";
    ++i;
  }
}

void printClusterIVs(const std::unordered_map<int, int> clusters,
                     std::ofstream &outfile) {
  size_t i = 0;
  for (const auto &it : clusters) {
    outfile << '"' << it.first << "\":";
    outfile << it.second;
    if (i != clusters.size() - 1) outfile << ", ";
    outfile << "\n";
    ++i;
  }
}

void printSingleFeat(const csaransh::featT &feats, std::ostream &outfile) {
  outfile << "\"dist\": ";
  outfile << "[";
  csaransh::writeStdAr(std::get<0>(feats), outfile);
  outfile << "],\n";
  outfile << "\"angle\": ";
  outfile << "[";
  csaransh::writeStdAr(std::get<1>(feats), outfile);
  outfile << "],\n";
  outfile << "\"adjNn2\": ";
  outfile << "[";
  csaransh::writeStdAr(std::get<2>(feats), outfile);
  outfile << "]\n";
}

void printFeats(const std::unordered_map<int, csaransh::featT> &feats,
                std::ostream &outfile) {
  size_t count = 0;
  for (const auto &it : feats) {
    outfile << '"' << it.first << "\":";
    outfile << "{";
    printSingleFeat(it.second, outfile);
    outfile << "}";
    if (count++ != feats.size() - 1) outfile << ", ";
    outfile << "\n";
  }
}

auto strSimulationCode(csaransh::SimulationCode code) {
  return (code == csaransh::SimulationCode::parcas)
             ? "parcas"
             : ((code == csaransh::SimulationCode::lammps) ? "lammps-xyz"
                                                           : "lammps-disp");
}

void csaransh::printJson(
    std::ofstream &outfile, const csaransh::Info &i, int id,
    const int &nDefects, const int &nClusters, const int &maxClusterSizeI,
    const int &maxClusterSizeV, const double &inClusterFractionI,
    const double &inClusterFractionV, const csaransh::DefectVecT &defects,
    const std::array<std::vector<double>, 2> &dists,
    const std::array<std::vector<double>, 2> &angles,
    const std::unordered_map<int, std::vector<int>> clusters,
    const std::unordered_map<int, int> clustersIV,
    const std::unordered_map<int, csaransh::featT> &feats) {
  auto printDefects = [&outfile](const csaransh::DefectVecT &d) {
    size_t count = 0;
    for (const auto &x : d) {
      outfile << "[" << std::get<0>(x)[0] << ", " << std::get<0>(x)[1] << ", "
              << std::get<0>(x)[2] << ", " << std::get<1>(x) << ", "
              << std::get<2>(x) << ", " << std::get<3>(x) << "]";
      if (count++ < (d.size() - 1)) outfile << ", ";
    }
  };
  outfile << "{";
  outfile << "\"infile\": \"" << i.infile << "\",\n"
          << "\"name\": \"" << i.name << "\",\n"
          << "\"id\": \"" << id << "\",\n"
          << "\"substrate\": \"" << i.substrate << "\",\n"
          << "\"simulationCode\": \"" << strSimulationCode(i.simulationCode)
          << "\",\n"
          << "\"boxSize\":" << i.boxSize << ",\n"
          << "\"energy\":" << i.energy << ",\n"
          << "\"ncell\":" << i.ncell << ",\n"
          << "\"rectheta\":" << i.rectheta << ",\n"
          << "\"recphi\":" << i.recphi << ",\n"
          << "\"xrec\":" << i.xrec << ",\n"
          << "\"yrec\":" << i.yrec << ",\n"
          << "\"zrec\":" << i.zrec << ",\n"
          << "\"latticeConst\":" << i.latticeConst << ",\n"
          << "\"origin\":" << i.origin << ",\n";
  outfile << "\"n_defects\":" << nDefects << ",\n"
          << "\"n_clusters\":" << nClusters << ",\n"
          << "\"max_cluster_size_I\":" << maxClusterSizeI << ",\n"
          << "\"max_cluster_size_V\":" << maxClusterSizeV << ",\n"
          << "\"max_cluster_size\":"
          << std::max(maxClusterSizeI, maxClusterSizeV) << ",\n"
          << "\"in_cluster_I\":" << inClusterFractionI << ",\n"
          << "\"in_cluster_V\":" << inClusterFractionV << ",\n"
          << "\"in_cluster\":"
          << (inClusterFractionI + inClusterFractionV) / 2.0 << ",\n";
  outfile << "\"coords\": [";
  printDefects(defects);
  outfile << "],\n";
  outfile << "\"clusters\": ";
  outfile << "{";
  printClusterIds(clusters, outfile);
  outfile << "}";
  outfile << ",\n";
  outfile << "\"clusterSizes\": ";
  outfile << "{";
  printClusterIVs(clustersIV, outfile);
  outfile << "}";
  outfile << ",\n";
  outfile << "\"features\": ";
  outfile << "{";
  printFeats(feats, outfile);
  outfile << "}";
  outfile << ",\n";
  outfile << "\"distancesI\": [";
  csaransh::writeVector(dists[0], outfile);
  outfile << "]";
  outfile << ",\n";
  outfile << "\"distancesV\": [";
  csaransh::writeVector(dists[1], outfile);
  outfile << "]";
  outfile << ",\n";
  outfile << "\"anglesI\": [";
  csaransh::writeVector(angles[0], outfile);
  outfile << "]";
  outfile << ",\n";
  outfile << "\"anglesV\": [";
  csaransh::writeVector(angles[1], outfile);
  outfile << "]\n";
  outfile << "}\n";
}