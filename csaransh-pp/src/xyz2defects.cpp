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

std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>>
getAtoms(csaransh::InputInfo &info, csaransh::ExtraInfo &extraInfo,
         const csaransh::Config &config) {
  using csaransh::Coords;
  using csaransh::strAr;
  using std::get;
  using std::string;
  using std::tuple;
  using std::vector;
  vector<Coords>
      atoms; // for all the atoms along with nearest closest sites and offset
  vector<tuple<Coords, double, Coords>>
      res; // for all the atoms along with nearest closest sites and offset
  std::ifstream infile{info.xyzFilePath};
  if (infile.bad() || !infile.is_open()) return res;
  const auto &latConst = info.latticeConst;
  if (info.ncell > 0) atoms.reserve(info.ncell * info.ncell * info.ncell * 2);
  std::string line;
  // read file and apply object
  csaransh::frameStatus fs = csaransh::frameStatus::prelude;
  Coords c;
  csaransh::lineStatus ls;
  while (std::getline(infile, line)) {
    std::tie(ls, c) = csaransh::getCoord(line, fs, info, extraInfo);
    if (ls == csaransh::lineStatus::coords &&
        fs == csaransh::frameStatus::inFrame) {
      atoms.emplace_back(c);
    } else if (ls == csaransh::lineStatus::inFrameCoords) {
      atoms.emplace_back(c);
    } else if (ls == csaransh::lineStatus::frameBorder) {
      fs = csaransh::frameStatus::inFrame;
      if (fs != csaransh::frameStatus::prelude) {
        atoms.clear(); // Ignoring the frame before this one
      }
    }
  }
  infile.close();
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
  res.reserve(atoms.size());
  auto origin = csaransh::Coords{{info.originX, info.originY, info.originZ}};
  auto obj = csaransh::AddOffset{info.latticeConst, info.structure, origin};
  std::transform(begin(atoms), end(atoms), std::back_inserter(res), obj);
  if (info.boxSize < 0.0) { info.boxSize = info.latticeConst * info.ncell; }
  return res;
}

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
csaransh::xyz2defects(const std::string &fname, csaransh::InputInfo &mainInfo,
                      csaransh::ExtraInfo &extraInfo,
                      const csaransh::Config &config) {
  auto atoms = getAtoms(mainInfo, extraInfo, config);
  if (atoms.empty())
    return std::make_pair(csaransh::DefectVecT{},
                          std::vector<std::array<int, 2>>{});
  return csaransh::atoms2defects(atoms, mainInfo, extraInfo, config);
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
std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
csaransh::atoms2defects(
    std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>> atoms,
    csaransh::InputInfo &info, csaransh::ExtraInfo &extraInfo,
    const csaransh::Config &config) {
  using csaransh::Coords;
  using std::get;
  using std::tuple;
  using std::vector;
  std::sort(begin(atoms), end(atoms));
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
  csaransh::NextExpected nextExpected{firstRow, maxes, maxesInitial};
  setCenter(extraInfo, firstRow, lastRow);
  // csaransh::Logger::inst().log_debug("Min: " + strAr(firstRow) + " coord: " +
  // strAr(std::get<2>(*minmax.first)));
  // csaransh::Logger::inst().log_debug("Max: " + strAr(lastRow) + " coord: " +
  // strAr(std::get<2>(*minmax.second))); csaransh::NextExpected
  // nextExpected2{firstRow, maxes, maxesInitial}; testIt(atoms, nextExpected2);
  std::tuple<double, Coords, Coords> pre;
  bool isPre = false;
  const auto latConst = info.latticeConst;
  const auto thresh = getThresh(
      info, config.thresholdFactor); // for rough interstitial threshold
  vector<tuple<double, Coords, Coords>>
      interThresh; // interstitials based on rough thresh
  vector<tuple<Coords, Coords, bool>> interstitials;
  vector<tuple<Coords, bool>> vacancies;
  vector<std::array<int, 2>> dumbbellTriplets;
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
    if (config.isAddThresholdInterstitials && curOffset > thresh) {
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
          vacancies.emplace_back(nextExpected.cur(), true);
          if (config.safeRunChecks &&
              vacancies.size() * extraFactor > atoms.size()) {
            if (!(Logger::inst().mode() & LogMode::info))
              Logger::inst().log_info("from \"" + info.xyzFilePath + "\"");
            Logger::inst().log_error(
                "not processing since too many vacancies, this might be due to "
                "wrong inputs like latticeConst, boxDim or corrupt xyz file. "
                "(v): " +
                std::to_string(vacancies.size()));
            return std::make_pair(DefectVecT{}, vector<std::array<int, 2>>{});
          }
        }
        nextExpected.increment();
      }
      nextExpected.increment();
    } else { // atom sits at the same lattice site again
      vecB res;
      if (isPre && std::get<1>(pre) == ar) {
        auto isPreReal = std::get<0>(pre) > curOffset;
        interstitials.emplace_back(std::get<1>(pre), std::get<2>(pre),
                                   isPreReal);
        vacancies.emplace_back(ar, false); // dummy vacancy added
        interstitials.emplace_back(
            ar, c,
            !isPreReal); // both interstitials for structures like dumbbells
        dumbbellTriplets.emplace_back(std::array<int, 2>{
            {int(interstitials.size() - 2), int(vacancies.size() - 1)}});
      } else {
        if (boundaryPred(ar, nextExpected.minCur(), nextExpected.maxCur(),
                         boundaryThresh) == false) {
          interstitials.emplace_back(ar, c, true);
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
        return std::make_pair(DefectVecT{}, vector<std::array<int, 2>>{});
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
      return std::make_pair(DefectVecT{}, vector<std::array<int, 2>>{});
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
      return std::make_pair(DefectVecT{}, vector<std::array<int, 2>>{});
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
        return std::make_pair(DefectVecT{}, std::move(dumbbellTriplets));
      }
    }
  }
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
  clean(interstitials, vacancies, info.latticeConst); // clean again
  for (const auto &it : interstitials) {
    defects.emplace_back(std::move(get<1>(it)), true, count++, get<2>(it));
  }
  for (const auto &it : vacancies) {
    defects.emplace_back(std::move(get<0>(it)), false, count++, get<1>(it));
  }
  for (auto &it : dumbbellTriplets) {
    it[1] += interstitials.size();
  }
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
    }
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
  return std::make_pair(std::move(defects), std::move(dumbbellTriplets));
}

std::vector<std::array<csaransh::Coords, 2>>
getDisplacedAtoms(const std::string &fname) {
  using csaransh::Coords;
  using std::get;
  using std::string;
  using std::tuple;
  using std::vector;
  vector<std::array<Coords, 2>> atoms;
  std::ifstream infile{fname};
  if (infile.bad() || !infile.is_open()) return atoms;
  // const auto latConst = info.latticeConst;
  std::string line;
  // read file and apply object
  csaransh::frameStatus fs = csaransh::frameStatus::prelude;
  std::array<csaransh::Coords, 2> c;
  csaransh::lineStatus ls;
  while (std::getline(infile, line)) {
    std::tie(ls, c) = csaransh::getCoordDisplaced(line);
    if (ls == csaransh::lineStatus::coords &&
        fs == csaransh::frameStatus::inFrame) {
      atoms.emplace_back(c);
    } else if (ls == csaransh::lineStatus::frameBorder) {
      fs = csaransh::frameStatus::inFrame;
      if (fs != csaransh::frameStatus::prelude) {
        atoms.clear(); // Ignoring the frame before this one
      }
    }
  }
  infile.close();
  return atoms;
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

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
csaransh::displaced2defects(const std::string &fname, double latticeConst) {
  auto atoms = getDisplacedAtoms(fname);
  return displacedAtoms2defects(atoms, latticeConst);
}

std::pair<csaransh::DefectVecT, std::vector<std::array<int, 2>>>
csaransh::displacedAtoms2defects(
    std::vector<std::array<csaransh::Coords, 2>> atoms, double latticeConst) {
  using csaransh::Coords;
  using std::get;
  using std::tuple;
  using std::vector;
  csaransh::DefectVecT inter, vac, defects;
  auto vcr = 0.3 * latticeConst;
  auto count = 0;
  // recombining interstitials
  for (auto it : atoms) {
    auto flag = true;
    for (auto jt : atoms) {
      auto disp = calcDist(it[1], jt[0]);
      if (disp < vcr) flag = false;
    }
    if (flag) inter.emplace_back(it[1], true, count++, true);
  }
  for (auto it : atoms) {
    auto flag = true;
    for (auto jt : atoms) {
      auto disp = calcDist(it[0], jt[1]);
      if (disp < vcr) flag = false;
    }
    if (flag) vac.emplace_back(it[0], false, count++, true);
  }
  cleanDisplaced(inter, vac, latticeConst);
  defects = std::move(inter);
  defects.insert(std::end(defects), std::begin(vac), std::end(vac));
  return std::make_pair(std::move(defects), std::vector<std::array<int, 2>>{});
}