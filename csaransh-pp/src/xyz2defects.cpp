#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>

#include <AddOffset.hpp>
#include <helper.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>

const csaransh::Coords &csaransh::NextExpected::increment() {
  csaransh::Coords &c = _cur;
  constexpr auto epsilon = std::numeric_limits<double>::epsilon();
  for (auto i : {2, 1, 0}) {
    if (c[i] < (_maxCur - epsilon)) {
      if (i > 0) {
        c[i] += 1.0;
      } else { // if (i == 0)
        c[i] += 0.5;
        if (_minCur == _min) {
          _minCur = _min + 0.5;
          _maxCur = _max;
        } else {
          _minCur = _min;
          _maxCur = _max - 0.5;
        }
      }
      for (auto j = i + 1; j <= 2; ++j) {
        c[j] = _minCur;
      }
      return c;
    }
  }
  if (_maxCur != _max) {
    _maxCur = _max;
    _minCur = _min + 0.5;
    c[0] = _maxCur;
    c[1] = _minCur;
    c[2] = _minCur;
  }
  return c;
}

// tells if two coordinates are equal within epsilon = 1e-6 range
auto cmpApprox(const std::array<double, 3> &c1,
               const std::array<double, 3> &c2) {
  constexpr auto epsilon = std::numeric_limits<double>::epsilon();
  // auto epsilon = 1e-6;
  for (auto i : {0, 1, 2}) {
    if (fabs(c1[i] - c2[i]) > epsilon) return (c1[i] > c2[i]) ? 1 : -1;
  }
  return 0;
}

// tells if a coordinate exists in vector of coordinates. The equality is
// measured by cmpApprox
auto existAnchorVac(const std::array<double, 3> &t,
                    const std::vector<std::array<double, 3>> &v) {
  auto it =
      std::lower_bound(begin(v), end(v), t, [](const std::array<double, 3> &a,
                                               const std::array<double, 3> &b) {
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

std::pair<csaransh::lineStatus, csaransh::Coords>
csaransh::getCoordLammps(const std::string &line) {
  // std::cout << "reading lammps coords\n";
  csaransh::Coords c;
  auto first = std::find_if(begin(line), end(line),
                            [](int ch) { return !std::isspace(ch); });
  if (first == std::end(line))
    return std::make_pair(csaransh::lineStatus::garbage,
                          c); // possibly blank line
  auto second =
      std::find_if(first, end(line), [](int ch) { return std::isspace(ch); });
  std::string word{first, second};
  if (word == "ITEM:") {
    return std::make_pair(csaransh::lineStatus::frameBorder, c);
  }
  for (auto i = 0; i < 3; ++i) {
    first = std::find_if(second, end(line),
                         [](int ch) { return !std::isspace(ch); });
    second =
        std::find_if(first, end(line), [](int ch) { return std::isspace(ch); });
    if (first >= second)
      return std::make_pair(csaransh::lineStatus::garbage, c);
    try {
      c[i] = std::stod(std::string{first, second});
    } catch (const std::invalid_argument &) {
      return std::make_pair(csaransh::lineStatus::garbage, c);
    } catch (const std::out_of_range &) {
      return std::make_pair(csaransh::lineStatus::garbage, c);
    }
  }
  return std::make_pair(csaransh::lineStatus::coords, c);
}

std::pair<csaransh::lineStatus, csaransh::Coords>
csaransh::getCoordParcas(const std::string &line) {
  csaransh::Coords c;
  auto first = std::find_if(begin(line), end(line),
                            [](int ch) { return !std::isspace(ch); });
  if (first == std::end(line))
    return std::make_pair(csaransh::lineStatus::garbage,
                          c); // possibly blank line
  if (std::isdigit(*first))
    return std::make_pair(csaransh::lineStatus::garbage,
                          c); // possibly first line with frame number
  auto second =
      std::find_if(first, end(line), [](int ch) { return std::isspace(ch); });
  std::string word{first, second};
  if (word == "Frame") {
    return std::make_pair(csaransh::lineStatus::frameBorder, c);
  }
  for (int i = 0; i < 3; ++i) {
    first = std::find_if(second, end(line),
                         [](int ch) { return !std::isspace(ch); });
    second =
        std::find_if(first, end(line), [](int ch) { return std::isspace(ch); });
    if (first >= second)
      return std::make_pair(csaransh::lineStatus::garbage, c);
    try {
      c[i] = std::stod(std::string{first, second});
    } catch (const std::invalid_argument &) {
      return std::make_pair(csaransh::lineStatus::garbage, c);
    } catch (const std::out_of_range &) {
      return std::make_pair(csaransh::lineStatus::garbage, c);
    }
  }
  return std::make_pair(csaransh::lineStatus::coords, c);
}

std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>>
getAtoms(const std::string &fname, const csaransh::Info &info) {
  using std::string;
  using std::vector;
  using std::tuple;
  using std::get;
  using csaransh::Coords;
  vector<tuple<Coords, double, Coords>>
      atoms; // for all the atoms along with nearest closest sites and offset
  std::ifstream infile{fname};
  if (infile.bad() || !infile.is_open()) return atoms;
  const auto latConst = info.latticeConst;
  auto o = info.origin;
  auto origin = std::array<double, 3>{{o, o, o}};
  auto obj = csaransh::AddOffset{latConst, info.structure, origin};
  atoms.reserve(info.ncell * info.ncell * info.ncell * 2);
  std::string line;
  // read file and apply object
  csaransh::frameStatus fs = csaransh::frameStatus::prelude;
  Coords c;
  csaransh::lineStatus ls;
  while (std::getline(infile, line)) {
    std::tie(ls, c) = (info.simulationCode == csaransh::SimulationCode::parcas)
                          ? csaransh::getCoordParcas(line)
                          : csaransh::getCoordLammps(line);
    if (ls == csaransh::lineStatus::coords &&
        fs == csaransh::frameStatus::inFrame) {
      atoms.emplace_back(obj(c));
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

csaransh::DefectVecT csaransh::xyz2defects(const std::string &fname,
                                           const csaransh::Info &info) {
  auto atoms = getAtoms(fname, info);
  if (atoms.empty()) return csaransh::DefectVecT{};
  return csaransh::atoms2defects(atoms, info.latticeConst);
}

// implements an O(log(N)) algorithm for detecting defects given a
// single frame file having xyz coordinates of all the atoms and input
// information
// Can be improved to O(N) as given here: https://arxiv.org/abs/1811.10923
csaransh::DefectVecT csaransh::atoms2defects(
    std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>> atoms,
    double latticeConst) {
  using std::vector;
  using std::tuple;
  using std::get;
  using csaransh::Coords;
  std::sort(begin(atoms), end(atoms));
  auto lastRow = get<0>(atoms[atoms.size() - 1]);
  csaransh::NextExpected nextExpected{get<0>(atoms[0])[0], lastRow[0]};
  std::tuple<double, Coords, Coords> pre;
  bool isPre = false;
  const auto latConst = latticeConst;
  const auto thresh = 0.3 * latConst; // for rough interstitial threshold
  vector<tuple<Coords, Coords>>
      interThresh; // interstitials based on rough thresh
  vector<tuple<Coords, Coords, bool>> interstitials;
  vector<tuple<Coords, bool>> vacancies;
  for (const auto &row : atoms) {
    // threshold
    if (get<1>(row) > thresh) {
      interThresh.emplace_back(get<0>(row), get<2>(row));
    }
    // clean
    Coords ar = get<0>(row);
    double x = get<1>(row);
    Coords c = std::get<2>(row);
    using vecB = std::vector<std::tuple<Coords, double, Coords>>;
    if (cmpApprox(ar, nextExpected.cur()) ==
        0) { // atom sits at correct lattice site
      nextExpected.increment();
      pre = std::make_tuple(x, ar, c);
      isPre = true;
    } else if (cmpApprox(ar, nextExpected.cur()) >
               0) { // atom sits at lattice site more than currently expected
      vecB res;
      while (cmpApprox(ar, nextExpected.cur()) > 0 &&
             cmpApprox(nextExpected.cur(), lastRow) < 0 &&
             !nextExpected.allMax()) {
        vacancies.emplace_back(nextExpected.cur(), true);
        nextExpected.increment();
      }
      nextExpected.increment();
    } else { // atom sits at the same lattice site again
      vecB res;
      if (isPre && std::get<1>(pre) == ar) {
        auto isPreReal = std::get<0>(pre) > x;
        interstitials.emplace_back(std::get<1>(pre), std::get<2>(pre),
                                   isPreReal);
        interstitials.emplace_back(
            ar, c,
            !isPreReal); // both interstitials for structures like dumbbells
        vacancies.emplace_back(ar, false); // dummy vacancy added
      } else {
        interstitials.emplace_back(ar, c, true);
      }
      isPre = false;
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
  clean(interstitials, vacancies, latticeConst); // clean again
  for (const auto &it : interstitials) {
    defects.emplace_back(std::move(get<1>(it)), true, count++, get<2>(it));
  }
  for (const auto &it : vacancies) {
    defects.emplace_back(std::move(get<0>(it)), false, count++, get<1>(it));
  }
  for (const auto &it : interThresh) {
    if (!existAnchorInter(std::get<0>(it), vacCleanSave)) {
      defects.emplace_back(anchor2coord(std::move(get<0>(it))), false, count++,
                           false);
      defects.emplace_back(std::move(get<1>(it)), true, count++, false);
    }
  }
  return defects;
}

std::pair<csaransh::lineStatus, std::array<csaransh::Coords, 2>>
csaransh::getCoordDisplaced(const std::string &line) {
  std::array<csaransh::Coords, 2> c;
  auto first = std::find_if(begin(line), end(line),
                            [](int ch) { return !std::isspace(ch); });
  if (first == std::end(line))
    return std::make_pair(csaransh::lineStatus::garbage,
                          c); // possibly blank line
  auto second =
      std::find_if(first, end(line), [](int ch) { return std::isspace(ch); });
  std::string word{first, second};
  if (word == "ITEM:") {
    return std::make_pair(csaransh::lineStatus::frameBorder, c);
  }
  for (auto j = 0; j < 2; ++j)
    for (auto i = 0; i < 3; ++i) {
      first = std::find_if(second, end(line),
                           [](int ch) { return !std::isspace(ch); });
      second = std::find_if(first, end(line),
                            [](int ch) { return std::isspace(ch); });
      if (first >= second)
        return std::make_pair(csaransh::lineStatus::garbage, c);
      try {
        c[j][i] = std::stod(std::string{first, second});
      } catch (const std::invalid_argument &) {
        return std::make_pair(csaransh::lineStatus::garbage, c);
      } catch (const std::out_of_range &) {
        return std::make_pair(csaransh::lineStatus::garbage, c);
      }
    }
  return std::make_pair(csaransh::lineStatus::coords, c);
}

std::vector<std::array<csaransh::Coords, 2>>
getDisplacedAtoms(const std::string &fname) {
  using std::string;
  using std::vector;
  using std::tuple;
  using std::get;
  using csaransh::Coords;
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
  using csaransh::DefectTWrap::isSurviving;
  using csaransh::DefectTWrap::coords;
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

csaransh::DefectVecT csaransh::displaced2defects(const std::string &fname,
                                                 double latticeConst) {
  auto atoms = getDisplacedAtoms(fname);
  return displacedAtoms2defects(atoms, latticeConst);
}

csaransh::DefectVecT csaransh::displacedAtoms2defects(
    std::vector<std::array<csaransh::Coords, 2>> atoms, double latticeConst) {
  using std::vector;
  using std::tuple;
  using std::get;
  using csaransh::Coords;
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
  return defects;
}