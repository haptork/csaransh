#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include <AddOffset.hpp>
#include <NextExpected.hpp>
#include <helper.hpp>
#include <logger.hpp>
#include <optimizeOrigin.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>
#include <xyzReader.hpp>

#include <iostream>

auto getThresh(const csaransh::InputInfo &info, const double &factor) {
  return factor * info.latticeConst;
}

// tells if two coordinates are equal within epsilon = 1e-6 range
auto cmpApprox(const std::array<double, 3> &c1,
               const std::array<double, 3> &c2) {
  // constexpr auto epsilon = std::numeric_limits<double>::epsilon();
  auto epsilon = 1e-2;
  for (auto i : {0, 1, 2}) {
    if (fabs(c1[i] - c2[i]) > epsilon) return (c1[i] > c2[i]) ? 1 : -1;
  }
  return 0;
}

// tells if a coordinate exists in vector of coordinates. The equality is
// measured by cmpApprox
auto existAnchorVac(const std::array<double, 3> &t,
                    const std::vector<std::array<double, 3>> &v) {
  auto it = std::lower_bound(
      begin(v), end(v), t,
      [](const std::array<double, 3> &a, const std::array<double, 3> &b) {
        if (cmpApprox(a, b) < 0) return true;
        return false;
      });
  if (it != std::end(v) && cmpApprox(*it, t) == 0) return true;
  return false;
}

// tells if a coordinate exists in vector of coordinates + bool. The equality is
// measured by cmpApprox
auto existAnchorInter(
    const std::array<double, 3> &t,
    const std::vector<std::tuple<std::array<double, 3>, bool>> &v) {
  auto temp = std::make_tuple(t, false);
  auto it =
      std::lower_bound(begin(v), end(v), temp,
                       [](const std::tuple<std::array<double, 3>, bool> &a,
                          const std::tuple<std::array<double, 3>, bool> &b) {
                         if (cmpApprox(std::get<0>(a), std::get<0>(b)) < 0)
                           return true;
                         return false;
                       });
  if (it != std::end(v) && cmpApprox(std::get<0>(*it), t) == 0) return true;
  return false;
}

// sets the surviving flag of an interstitial and vacancy if they are closer
// than a threshold
auto clean(
    std::vector<std::tuple<csaransh::Coords, csaransh::Coords, bool>> &inter,
    std::vector<std::tuple<csaransh::Coords, bool>> &vac, double latticeConst) {
  const auto &thresh = latticeConst; // * sqrt(3) / 2;
  for (size_t i = 0; i < vac.size(); ++i) {
    if (!std::get<1>(vac[i])) continue;
    auto min = thresh + 1e-6;
    size_t minj = 0;
    for (size_t j = 0; j < inter.size(); ++j) {
      if (!std::get<2>(inter[j])) continue;
      auto dist =
          csaransh::calcDist(std::get<0>(vac[i]), std::get<1>(inter[j]));
      if (dist < min) {
        min = dist;
        minj = j;
      }
    }
    if (min < thresh) {
      std::get<1>(vac[i]) = false;
      std::get<2>(inter[minj]) = false;
    }
  }
}

std::pair<csaransh::xyzFileStatus, std::array<std::vector<csaransh::Coords>, 2>>
getDisplacedAtomsTime(csaransh::InputInfo &info, csaransh::ExtraInfo &extraInfo,
         const csaransh::Config &config, std::istream &infile, csaransh::frameStatus &fs) {
  using csaransh::Coords;
  using std::get;
  using std::string;
  using std::tuple;
  using std::vector;
  std::pair<csaransh::xyzFileStatus, std::array<std::vector<csaransh::Coords>, 2>> res;
  auto &atoms = res.second;
  //std::array<std::vector<csaransh::Coords>, 2> atoms;
  // const auto latConst = info.latticeConst;
  std::string line;
  // read file and apply object
  std::array<csaransh::Coords, 2> c;
  csaransh::lineStatus ls;
  auto origin = std::array<double,3>{{info.originX, info.originY, info.originZ}};
  auto latConst = info.latticeConst;
  auto obj = csaransh::AddOffset{latConst, info.structure, origin};
  auto fn = [&latConst] (double x) { return x * latConst; };
  res.first = csaransh::xyzFileStatus::eof;
  while (std::getline(infile, line)) {
    std::tie(ls, c) = csaransh::getCoordDisplaced(line);
    if (ls == csaransh::lineStatus::coords &&
        fs == csaransh::frameStatus::inFrame) {
      auto vacC = std::get<0>(obj(c[0]));
      std::transform(begin(vacC), end(vacC), begin(vacC), fn);
      atoms[0].emplace_back(std::move(vacC));
      atoms[1].emplace_back(c[1]);
    } else if (ls == csaransh::lineStatus::frameBorder) {
      if (fs != csaransh::frameStatus::prelude) {
        fs = csaransh::frameStatus::inFrame;
        if (config.allFrames && !atoms[0].empty()) {
          res.first = csaransh::xyzFileStatus::reading;
          break;
        }
        atoms[0].clear();
        atoms[1].clear();
      }
      fs = csaransh::frameStatus::inFrame;
    }
  }
  return res;
}

std::pair<csaransh::xyzFileStatus, std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>>>
getAtomsTime(csaransh::InputInfo &info, csaransh::ExtraInfo &extraInfo,
         const csaransh::Config &config, std::istream &infile, csaransh::frameStatus &fs) {
  using csaransh::Coords;
  using csaransh::strAr;
  using std::get;
  using std::string;
  using std::tuple;
  using std::vector;
  vector<Coords>
      atoms; // for all the atoms along with nearest closest sites and offset
  std::pair<csaransh::xyzFileStatus, vector<tuple<Coords, double, Coords>>>
      res; // for all the atoms along with nearest closest sites and offset
  const auto &latConst = info.latticeConst;
  if (info.ncell > 0) atoms.reserve(info.ncell * info.ncell * info.ncell * 2);
  std::string line;
  // read file and apply object
  Coords c;
  csaransh::lineStatus ls;
  res.first = csaransh::xyzFileStatus::eof;
  while (std::getline(infile, line)) {
    std::tie(ls, c) = csaransh::getCoord(line, fs, info, extraInfo);
    if (ls == csaransh::lineStatus::coords &&
        fs == csaransh::frameStatus::inFrame) {
      atoms.emplace_back(c);
    } else if (ls == csaransh::lineStatus::inFrameCoords) {
      atoms.emplace_back(c);
    } else if (ls == csaransh::lineStatus::frameBorder) {
      if (fs != csaransh::frameStatus::prelude) {
        fs = csaransh::frameStatus::inFrame;
        if (config.allFrames && !atoms.empty()) {
          res.first = csaransh::xyzFileStatus::reading;
          break;
        }
        atoms.clear();
      }
      fs = csaransh::frameStatus::inFrame;
        //atoms.clear(); // Ignoring the frame before this one
    }
  }
  if (atoms.empty()) return res;
  // assuming bcc structure TODO: for fcc
  info.ncell = std::round(std::cbrt(atoms.size() / 2.0));
  auto secLatConst = (info.boxSize > 0.0) ? info.boxSize / info.ncell : -1.0;
  if (secLatConst > 0.0 && std::fabs(info.latticeConst - secLatConst) < 1e-3) {
    secLatConst = -1.0;
  }
  if (info.latticeConst < 0.0) {
    info.latticeConst = secLatConst;
    secLatConst = -1.0;
  }
  std::vector<std::tuple<csaransh::Coords, double, std::string, int>> combos;
  if (info.originType > 0) {
    auto originEstimated = csaransh::estimateOrigin(atoms, info.latticeConst);
    combos.emplace_back(
        csaransh::Coords{
            {originEstimated[0], originEstimated[1], originEstimated[2]}},
        info.latticeConst, "estimated origin, given latConst", 0);
    if (secLatConst > 0.0) {
      auto originEstimatedSec = csaransh::estimateOrigin(atoms, secLatConst);
      combos.emplace_back(
          csaransh::Coords{{originEstimatedSec[0], originEstimatedSec[1],
                            originEstimatedSec[2]}},
          secLatConst, "estimated originSec, boxSize/ncell latconst", 0);
    }
  }
  if ((secLatConst > 0.0 && info.originType == 0) || info.originType == 2) {
    combos.emplace_back(
        csaransh::Coords{{info.originX, info.originY, info.originZ}},
        info.latticeConst, "given origin, given latconst", 0);
    if (secLatConst > 0.0)
      combos.emplace_back(
          csaransh::Coords{{info.originX, info.originY, info.originZ}},
          secLatConst, "given origin, boxSize/ncell latconst", 0);
  }
  if (!combos.empty()) {
    auto leastIndex = 0;
    if (combos.size() > 1) {
      auto leastInterThresh = std::numeric_limits<int>::max();
      auto thresh = getThresh(info, config.thresholdFactor);
      for (size_t i = 0; i < combos.size(); i++) {
        auto curOrigin = std::get<0>(combos[i]);
        auto curLatConst = std::get<1>(combos[i]);
        auto obj = csaransh::AddOffset{curLatConst, info.structure, curOrigin};
        std::get<3>(combos[i]) = std::count_if(
            begin(atoms), end(atoms), [&obj, thresh](const auto &it) {
              return std::get<1>(obj(it)) > thresh;
            });
        csaransh::Logger::inst().log_debug(
            std::get<2>(combos[i]) + " origin, latconst " + strAr(curOrigin) +
            ", " + std::to_string(curLatConst) + " : nThreshold " +
            std::to_string(std::get<3>(combos[i])));
        if (std::get<3>(combos[i]) < leastInterThresh) {
          leastIndex = i;
          leastInterThresh = std::get<3>(combos[i]);
        }
      }
    }
    info.originX = std::get<0>(combos[leastIndex])[0];
    info.originY = std::get<0>(combos[leastIndex])[1];
    info.originZ = std::get<0>(combos[leastIndex])[2];
    info.latticeConst = std::get<1>(combos[leastIndex]);
  }
  res.second.reserve(atoms.size());
  auto origin = csaransh::Coords{{info.originX, info.originY, info.originZ}};
  auto obj = csaransh::AddOffset{info.latticeConst, info.structure, origin};
  // std::cout << "latInfo: " << info.latticeConst << ", " << info.structure << ", " << origin[0] << '\n';
  std::transform(begin(atoms), end(atoms), std::back_inserter(res.second), obj);
  // std::cout << "\natoms1: " << atoms[0][0] << " | " << atoms[atoms.size() - 1][0] << '\n';
  // std::cout << "\natoms res: " << std::get<0>(res.second[0])[0] << " | " << std::get<0>(res.second[res.second.size() - 1])[0] << '\n';
  if (info.boxSize < 0.0) { info.boxSize = info.latticeConst * info.ncell; }
  return res;
}

csaransh::DefectRes
csaransh::xyz2defectsTime(csaransh::InputInfo &mainInfo,
                      csaransh::ExtraInfo &extraInfo,
                      const csaransh::Config &config, std::istream &infile, csaransh::frameStatus &fs) {
  auto atoms = getAtomsTime(mainInfo, extraInfo, config, infile, fs);
  if (atoms.second.empty())
    return std::make_tuple(atoms.first, ErrorStatus::noError, csaransh::DefectVecT{}, std::vector<int>{});
  return csaransh::atoms2defects(atoms, mainInfo, extraInfo, config);
}

csaransh::DefectRes
csaransh::displaced2defectsTime(csaransh::InputInfo &mainInfo,
                      csaransh::ExtraInfo &extraInfo,
                      const csaransh::Config &config, std::istream &infile, csaransh::frameStatus &fs) {
  auto atoms = getDisplacedAtomsTime(mainInfo, extraInfo, config, infile, fs);
  if (atoms.second.empty())
    return std::make_tuple(atoms.first, ErrorStatus::noError, csaransh::DefectVecT{}, std::vector<int>{});
  return csaransh::displacedAtoms2defects(atoms, mainInfo.latticeConst);
}

/*
void testIt(std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>>
atoms, csaransh::NextExpected n) { std::ofstream f{"junk_got.txt"}; for (auto it
: atoms) { for (auto jt: std::get<0>(it)) { f << jt << " ";
    }
    f << " | ";
    for (auto jt: std::get<2>(it)) {
      f << jt << " ";
    }
    f << '\n';
  }
  f.close();
  std::ofstream fe{"junk_expected.txt"};
  for (auto jt: n.cur()) {
    fe << jt << " ";
  }
  fe << '\n';
  while (!n.allMax()) {
   n.increment();
   for (auto jt: n.cur()) {
      fe << jt << " ";
    }
    fe << " | ";
   for (auto jt: n.minCur()) {
      fe << jt << " ";
    }
    fe << " | ";
    for (auto jt: n.maxCur()) {
      fe << jt << " ";
    }
    fe << '\n';
   }
  fe.close();
}
*/

csaransh::Coords csaransh::getInitialMax(const csaransh::Coords &origin,
                                         const csaransh::Coords &maxes) {
  auto maxes1 = maxes;
  constexpr auto epsilon = 1e-2;
  for (auto i = 0; i < 3; i++) {
    if (maxes[i] - origin[i] - (int(maxes[i] - origin[i])) < epsilon) {
      maxes1[i] = maxes[i];
      // maxes2[i] = maxes[i] - 0.5;
    } else {
      maxes1[i] = maxes[i] - 0.5;
      // maxes2[i] = maxes[i];
    }
  }
  return maxes1;
}

void setCenter(csaransh::ExtraInfo &extraInfo, csaransh::Coords minC,
               csaransh::Coords maxC) {
  if (!extraInfo.isPkaGiven) {
    extraInfo.xrec = (maxC[0] - minC[0]) / 2.0 + minC[0];
    extraInfo.yrec = (maxC[1] - minC[1]) / 2.0 + minC[1];
    extraInfo.zrec = (maxC[2] - minC[2]) / 2.0 + minC[2];
  }
}

// implements an O(log(N)) algorithm for detecting defects given a
// single frame file having xyz coordinates of all the atoms and input
// information
// Can be improved to O(N) as given here: https://arxiv.org/abs/1811.10923
csaransh::DefectRes csaransh::atoms2defects(
    std::pair<csaransh::xyzFileStatus, std::vector<csaransh::offsetCoords>> stAtoms,
    csaransh::InputInfo &info, csaransh::ExtraInfo &extraInfo,
    const csaransh::Config &config) {
  using csaransh::Coords;
  using std::get;
  using std::tuple;
  using std::vector;
  auto &atoms = stAtoms.second;
  std::sort(begin(atoms), end(atoms));
  // std::cout << "\natoms: " << std::get<0>(atoms[0])[0] << " | " << std::get<0>(atoms[atoms.size() - 1])[0] << '\n';
  const auto minmax = std::minmax_element(
      begin(atoms), end(atoms), [](const auto &ao, const auto &bo) {
        const auto &a = std::get<0>(ao);
        const auto &b = std::get<0>(bo);
        return (a[0] + a[1] + a[2]) < (b[0] + b[1] + b[2]);
      });
  auto firstRow = std::get<0>(*(minmax.first));
  auto lastRow = std::get<0>(*(minmax.second));
  auto maxes = lastRow;
  auto maxesInitial = csaransh::getInitialMax(firstRow, maxes);
  // csaransh::Logger::inst().log_debug("Atoms: " + std::to_string(atoms.size()));
  // std::cout << "\nfirstRow: " << firstRow[0] << '\n';
  // std::cout << "\nmaxes: " << maxes[0] << ", " << maxes[1] << ", " << maxes[2] << '\n';
  // std::cout << "\nmaxesInitial: " << maxesInitial[0] << ", " << maxesInitial[1] << ", " << maxesInitial[2] << '\n';
  csaransh::NextExpected nextExpected{firstRow, maxes, maxesInitial};
  setCenter(extraInfo, firstRow, lastRow);
   // csaransh::Logger::inst().log_debug("Min: " + strAr(firstRow) + " coord: " +
   // strAr(std::get<2>(*minmax.first)));
   // csaransh::Logger::inst().log_debug("Max: " + strAr(lastRow) + " coord: " +
   // strAr(std::get<2>(*minmax.second)));
   //csaransh::NextExpected nextExpected2{firstRow, maxes, maxesInitial}; testIt(atoms, nextExpected2);
  std::tuple<double, Coords, Coords> pre;
  bool isPre = false;
  const auto latConst = info.latticeConst;
  const auto thresh = getThresh(
      info, config.thresholdFactor); // for rough interstitial threshold
  vector<tuple<double, Coords, Coords>>
      interThresh; // interstitials based on rough thresh
  vector<tuple<Coords, Coords, bool>> interstitials;
  vector<tuple<Coords, bool>> vacancies;
  std::vector<int> vacSias;
  vector<tuple<Coords, Coords, bool>> freeInterstitials;
  const auto boundaryThresh = (config.isIgnoreBoundaryDefects) ? 0.501 : 0.00;
  auto boundaryPred = [](Coords ar, Coords min, Coords max, double thresh) {
    for (size_t i = 0; i < ar.size(); i++) {
      if (ar[i] - thresh < min[i] || ar[i] + thresh > max[i]) { return true; }
    }
    return false;
  };
  const auto &extraFactor = config.extraDefectsSafetyFactor;
  for (const auto &row : atoms) {
    Coords ar = get<0>(row);
    double curOffset = get<1>(row);
    Coords c = std::get<2>(row);
    const auto cmpRes = cmpApprox(ar, nextExpected.cur());
    if (nextExpected.allMax()) break;
    // threshold
    // std::cout << ar[0] << ", " << config.isAddThresholdDefects << '\n';
    if (config.isAddThresholdDefects && curOffset > thresh) {
      interThresh.emplace_back(curOffset, ar, c);
    }
    // clean
    using vecB = std::vector<std::tuple<Coords, double, Coords>>;
    if (cmpRes == 0) { // atom at correct lattice site
      nextExpected.increment();
      pre = std::make_tuple(curOffset, ar, c);
      isPre = true;
    } else if (cmpRes > 0) { // at site after expected
      vecB res;
      while (cmpApprox(ar, nextExpected.cur()) > 0 &&
             cmpApprox(nextExpected.cur(), lastRow) <= 0 &&
             !nextExpected.allMax()) {
        if (!boundaryPred(nextExpected.cur(), nextExpected.minCur(),
                          nextExpected.maxCur(), boundaryThresh)) {
          vacSias.emplace_back(interstitials.size());
          vacancies.emplace_back(nextExpected.cur(), true);
          //csaransh::Logger::inst().log_debug("vac: " + strAr(nextExpected.cur()) + " coord: " + strAr(ar)); 
          if (config.safeRunChecks &&
              vacancies.size() * extraFactor > atoms.size()) {
            if (!(Logger::inst().mode() & LogMode::info))
              Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
            Logger::inst().log_error(
                "not processing since too many vacancies, this might be due to "
                "wrong inputs like latticeConst, boxDim or corrupt xyz file. "
                "(v): " +
                std::to_string(vacancies.size()));
            return std::make_tuple(stAtoms.first, ErrorStatus::vacOverflow, DefectVecT{}, vector<int>{});
          }
        }
        nextExpected.increment();
      }
      nextExpected.increment();
    } else { // atom sits at the same lattice site again
      vecB res;
      if (isPre && std::get<1>(pre) == ar) {
        auto isPreReal = std::get<0>(pre) > curOffset;
        vacancies.emplace_back(ar, false); // dummy vacancy added
        interstitials.emplace_back(std::get<1>(pre), std::get<2>(pre),
                                   isPreReal);
        interstitials.emplace_back(
            ar, c,
            !isPreReal); // both interstitials for structures like dumbbells
        vacSias.push_back(interstitials.size());
      } else {
        if (boundaryPred(ar, nextExpected.minCur(), nextExpected.maxCur(),
                         boundaryThresh) == false) {
          if (vacSias.size() > 0 && cmpApprox(std::get<1>(pre), ar) == 0) {
            interstitials.emplace_back(ar, c, true);
            vacSias[vacSias.size() - 1] = interstitials.size();
          } else {
            freeInterstitials.emplace_back(ar, c, true);
            // std::cout << "\n interstitial wo pre: " << c[0] << c[1] << c[2] << std::endl; 
          }
        }
      }
      if (config.safeRunChecks &&
          interstitials.size() * extraFactor > atoms.size()) {
        if (!(Logger::inst().mode() & LogMode::info))
          Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
        Logger::inst().log_error(
            "not processing since too many interstitials, this might be due to "
            "wrong inputs like latticeConst, boxDim or corrupt xyz file. "
            "(i): " +
            std::to_string(interstitials.size()));
        return std::make_tuple(stAtoms.first, ErrorStatus::siaOverflow, DefectVecT{}, vector<int>{});
      }
      isPre = false;
    }
  }
  if (config.safeRunChecks) {
    if (interstitials.size() * extraFactor > atoms.size() ||
        vacancies.size() * extraFactor > atoms.size()) {
      if (!(Logger::inst().mode() & LogMode::info))
        Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
      Logger::inst().log_error(
          "not processing since too many interstitials and vacancies, this may "
          "be due to wrong inputs like latticeConst, boxDim or corrupt xyz "
          "file. (i, v): " +
          std::to_string(interstitials.size()) + ", " +
          std::to_string(vacancies.size()));
      return std::make_tuple(stAtoms.first, ErrorStatus::defectOverflow, DefectVecT{}, vector<int>{});
    }
    if (interThresh.size() > extraFactor * interstitials.size()) {
      if (!(Logger::inst().mode() & LogMode::info))
        Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
      Logger::inst().log_error(
          "not processing since interThresh is too big, this may be due to "
          "inputs like latticeConst, boxDim or corrupt xyz file.  (i, v, "
          "interThresh): " +
          std::to_string(interstitials.size()) + ", " +
          std::to_string(vacancies.size()) + ", " +
          std::to_string(interThresh.size()));
      return std::make_tuple(stAtoms.first, ErrorStatus::threshOverflow, DefectVecT{}, vector<int>{});
    }
    if (interstitials.size() != vacancies.size()) {
      if ((int(interstitials.size()) / int(vacancies.size())) > extraFactor) {
        if (!(Logger::inst().mode() & LogMode::info))
          Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
        csaransh::Logger::inst().log_warning(
            "interstitails and vacancies have different sizes (i, v, "
            "interTresh): " +
            std::to_string(interstitials.size()) + ", " +
            std::to_string(vacancies.size()) + ", " +
            std::to_string(interThresh.size()));
      } else {
        csaransh::Logger::inst().log_debug(
            "interstitails and vacancies have different sizes (i, v, "
            "interThresh): " +
            std::to_string(interstitials.size()) + ", " +
            std::to_string(vacancies.size()) + ", " +
            std::to_string(interThresh.size()));
      }
      if (interstitials.size() * (extraFactor / 10) < vacancies.size() ||
          vacancies.size() * (extraFactor / 10) < interstitials.size()) {
        if (!(Logger::inst().mode() & LogMode::info))
          Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
        csaransh::Logger::inst().log_error(
            "not processing since the difference is too big (i, v, "
            "interThresh): " +
            std::to_string(interstitials.size()) + ", " +
            std::to_string(vacancies.size()) + ", " +
            std::to_string(interThresh.size()));
        return std::make_tuple(stAtoms.first, ErrorStatus::siaVacDiffOverflow, DefectVecT{}, vector<int>{});
      }
    }
  }
  // std::cout<<"\ninterstitials: " << interstitials.size() << '\n';
  // std::cout<<"\nvacancies: " << vacancies.size() << '\n';
  // std::cout<<"\ninterThresh: " << interThresh.size() << '\n';
  atoms.clear();
  csaransh::DefectVecT defects;
  defects.reserve(2 * vacancies.size());
  auto vacCleanSave = vacancies;
  auto anchor2coord = [&latConst](Coords c) {
    for (auto &x : c)
      x *= latConst;
    return c;
  };
  auto count = 1;
  for (auto &it : vacancies) {
    for (auto &x : std::get<0>(it))
      x *= latConst;
  }
  /*
  std::cout <<"\nis: \n";
  for(auto &it: interstitials) {
    std::cout<<get<1>(it)[0] << ", " << get<1>(it)[1] << '\n';
  }
  std::cout <<"\nvs: \n";
  for(auto &it: vacancies) {
    std::cout<<get<0>(it)[0] << ", " << get<0>(it)[1] << '\n';
  }
  std::cout <<"\nvacSias: \n";
  for(auto &it: vacSias) {
    std::cout<<it <<'\n';
  }
  */
  for (auto it: freeInterstitials) {
    interstitials.push_back(it);
  }
  clean(interstitials, vacancies, info.latticeConst); // clean again
  std::vector<int> vacSiasNu;
  for (auto i = 0; i < vacancies.size(); i++) {
    auto &it = vacancies[i];
    defects.emplace_back(std::move(std::get<0>(it)), false, count++, std::get<1>(it));
    //std::cout << "I is " << i << ": ";
    for (auto j = (i > 0) ? vacSias[i-1] : 0; j < vacSias[i]; j++) {
      //std::cout << j << ", ";
      auto &jt = interstitials[j];
      defects.emplace_back(std::move(get<1>(jt)), true, count++, get<2>(jt));
    }
    //std::cout << '\n';
    vacSiasNu.push_back(defects.size());
  }
  /*
  std::cout <<"\ndefects: \n";
  for(auto &it: defects) {
    std::cout<<get<0>(it)[0] << ", " << get<0>(it)[1] << '\n';
  }
  */
  int extraDefects = 0;
  int maxExtraDefects = interstitials.size() * 3;
  if (maxExtraDefects > interThresh.size())
    maxExtraDefects = interThresh.size();
  std::nth_element(begin(interThresh), begin(interThresh) + maxExtraDefects,
                   end(interThresh), [](const auto &a, const auto &b) {
                     return std::get<0>(a) > std::get<0>(b);
                   });
  for (const auto &it : interThresh) {
    if (!existAnchorInter(std::get<1>(it), vacCleanSave)) {
      extraDefects++;
      if (config.safeRunChecks && extraDefects > maxExtraDefects) continue;
      defects.emplace_back(anchor2coord(std::move(get<1>(it))), false, count++,
                           false);
      defects.emplace_back(std::move(get<2>(it)), true, count++, false);
      vacSiasNu.emplace_back(defects.size());
    }
  }
  for (const auto &jt : freeInterstitials) {
    defects.emplace_back(std::move(get<1>(jt)), true, count++, get<2>(jt));
  }
  if (config.safeRunChecks && extraDefects > interstitials.size()) {
    if (extraDefects > maxExtraDefects) {
      Logger::inst().log_warning(
          "Some threshold based defects ignored. Too many threshold based "
          "defects. (i, v, extraI): " +
          std::to_string(interstitials.size()) + ", " +
          std::to_string(vacancies.size()) + ", " +
          std::to_string(extraDefects));
    } else {
      Logger::inst().log_debug(
          "Too many threshold based defects. (i, v, extraI): " +
          std::to_string(interstitials.size()) + ", " +
          std::to_string(vacancies.size()) + ", " +
          std::to_string(extraDefects));
    }
  }
  return std::make_tuple(stAtoms.first, ErrorStatus::noError, std::move(defects), std::move(vacSiasNu));
}

auto cleanDisplaced(csaransh::DefectVecT &inter, csaransh::DefectVecT &vac,
                    double latticeConst) {
  using csaransh::DefectTWrap::coords;
  using csaransh::DefectTWrap::isSurviving;
  auto thresh = latticeConst; // * sqrt(3) / 2;
  for (size_t i = 0; i < vac.size(); ++i) {
    auto min = thresh + 1e-6;
    size_t minj = 0;
    for (size_t j = 0; j < inter.size(); ++j) {
      if (!isSurviving(inter[j])) continue;
      auto dist = csaransh::calcDist(coords(vac[i]), coords(inter[j]));
      if (dist < min) {
        min = dist;
        minj = j;
      }
    }
    if (min < thresh) {
      isSurviving(vac[i], false);
      isSurviving(inter[minj], false);
    }
  }
}

csaransh::DefectRes csaransh::displacedAtoms2defects(
    std::pair<csaransh::xyzFileStatus, std::array<std::vector<csaransh::Coords>, 2>> statoms, double latticeConst) {
  using csaransh::Coords;
  using std::get;
  using std::tuple;
  using std::vector;
  csaransh::DefectVecT inter, vac, defects;
  auto &atoms = statoms.second;
  constexpr auto epsilon = 1e-4;
  const auto nn = (std::sqrt(3) * latticeConst) / 2 + epsilon;
  const auto threshDist = 0.345 * latticeConst + 1e-4;
  const auto threshDistSqr = threshDist * threshDist;
  const auto recombDist = latticeConst;//latticeConst; //0.345 * latticeConst + 1e-4;
  const auto recombDistSqr = recombDist * recombDist;
  const auto vacGroupDist = nn;
  const auto vacGroupDistSqr = vacGroupDist * vacGroupDist;
  const auto maxSqrDist = (recombDistSqr > vacGroupDistSqr) ? recombDistSqr : vacGroupDistSqr;
  std::vector<int> freeIs;
  const auto sortHelper = [](const std::array<double, 3> &c1, const std::array<double, 3> &c2) {
    return cmpApprox(c1, c2) < 0;
  };
  sort( atoms[0].begin(), atoms[0].end(), sortHelper);
  const auto areEq = [](const std::array<double, 3> &c1, const std::array<double, 3> &c2) {
    return cmpApprox(c1, c2) == 0;
  };
  atoms[0].erase(unique(atoms[0].begin(), atoms[0].end(), areEq), atoms[0].end());
  std::vector<std::vector<std::pair<double, int>>> vacSias(atoms[0].size());
  std::vector<std::vector<int>> siaVacsFull(atoms[1].size());
  for (auto is = 0; is < atoms[1].size(); is++) {
    auto minDist = maxSqrDist;
    auto minDistIndex = -1;
    for (auto iv = 0; iv < atoms[0].size(); iv++) {
      auto disp = calcDistSqr(atoms[1][is], atoms[0][iv]);
      if (disp < maxSqrDist) siaVacsFull[is].push_back(iv);// = disp;
      if (disp < minDist) {
        minDist = disp;
        minDistIndex = iv;
      }
    }
    if (minDistIndex >=0) {
      vacSias[minDistIndex].push_back(std::make_pair(minDist, is));
    } else {
      freeIs.push_back(is);
    }
  }
  for (auto i = 0; i < vacSias.size(); i++) {
    std::sort(begin(vacSias[i]), end(vacSias[i]));
  }
  /*
  std::cout << "siaFull: \n";
  for (auto is = 0; is < siaVacsFull.size(); is++) {
    for (const auto &jt : atoms[1][is]) std::cout << jt << ", ";
    std::cout << ": \n";
    for (const auto &kt : siaVacsFull[is]) {
      auto iv = kt;
      if (iv < 0) continue;
      std::cout << "\t";
      for (const auto &jt : atoms[0][iv]) std::cout << jt << ", ";
      std::cout << "\n";
    }
    std::cout << "---\n";
  }
  std::cout << "vacSias before : \n";
  for (auto iv = 0; iv < vacSias.size(); iv++) {
    for (const auto &jt : atoms[0][iv]) std::cout << jt << ", ";
    std::cout << ": \n";
    for (const auto &kt : vacSias[iv]) {
      auto is = kt.second;
      if (is < 0) continue;
      std::cout << "\t";
      for (const auto &jt : atoms[1][is]) std::cout << jt << ", ";
      std::cout << "\n";
    }
    std::cout << "---\n";
  }
  */
  for (auto iv = 0; iv < vacSias.size(); iv++) {
    for (auto j = 1; j < vacSias[iv].size(); j++) {
      auto is = vacSias[iv][j].second;
      for (auto vac : siaVacsFull[is]) {
        if (vac == iv || !vacSias[vac].empty()) continue;
        auto disp = calcDistSqr(atoms[1][is], atoms[0][vac]);
        vacSias[vac].push_back(std::make_pair(disp, is));
        vacSias[iv][j].second = -1;
      }
    }
  }
 
  /*
  std::cout << "atoms[0]: \n";
  for (auto it : atoms[0]) {
    for (auto jt : it) std::cout << jt << ", ";
    std::cout << '\n';
  }
  std::cout << "atoms[1]: \n";
  for (auto it : atoms[1]) {
    for (auto jt : it) std::cout << jt << ", ";
    std::cout << '\n';
  }
  auto temp = 0;
  std::cout << "vacSias: \n";
  for (auto it : vacSias) {
    for (auto jt : it) std::cout << jt.first << ", " << jt.second << "; ";
    std::cout << temp++ << "---\n";
  }
  std::cout << "vacSias after : \n";
  for (auto iv = 0; iv < vacSias.size(); iv++) {
    for (const auto &jt : atoms[0][iv]) std::cout << jt << ", ";
    std::cout << ": \n";
    for (const auto &kt : vacSias[iv]) {
      std::cout << "\t";
      auto is = kt.second;
      if (is < 0) continue;
      for (const auto &jt : atoms[1][is]) std::cout << jt << ", ";
      std::cout << "\n";
    }
    std::cout << "---\n";
  }
  */
  std::vector<int> latticeSiteGroups;
  latticeSiteGroups.reserve(vacSias.size());
  for (auto i = 0; i < vacSias.size(); i++) {
    if (vacSias[i].size() == 1 && vacSias[i][0].first < threshDistSqr) continue;
    std::sort(begin(vacSias[i]), end(vacSias[i]));
    bool isAnnihilated = (!vacSias[i].empty() && vacSias[i][0].first < recombDistSqr);
    //std::cout << i << ": " << vacSias[i].size() << '\n' << std::flush;
    defects.emplace_back(atoms[0][i], false, defects.size() + 1, !isAnnihilated);  // vacancy
    if (!vacSias[i].empty()) defects.emplace_back(atoms[1][vacSias[i][0].second], true, defects.size() + 1, !isAnnihilated); // interstitial
    auto j = 1;
    for (; j < vacSias[i].size() && vacSias[i][j].first < vacGroupDistSqr; j++) {
      auto is = vacSias[i][j].second;
      if (is < 0) continue;
      defects.emplace_back(atoms[1][is], true, defects.size() + 1, true); // interstitial
    }
    for (;j < vacSias[i].size(); j++) {
      auto is = vacSias[i][j].second;
      if (is < 0) continue;
      freeIs.push_back(is);
    }
    //std::cout << defects.size() << '\n';
    latticeSiteGroups.push_back(defects.size());
  }
  for (auto it : freeIs) {
    defects.emplace_back(atoms[1][it], true, defects.size() + 1, true); // interstitial
  }
  /*
  std::cout << "latticeGroups: \n";
  for (auto it : latticeSiteGroups) {
    std::cout << it << ", ";
  }
  std::cout << '\n';
  */
  //std::cout <<"finnal defects size: " << defects.size() << '\n';
  return std::make_tuple(statoms.first, csaransh::ErrorStatus::noError, std::move(defects), std::move(latticeSiteGroups));
}