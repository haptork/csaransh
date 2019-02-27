/*!
 * */
#include <algorithm>
#include <array>
#include <iostream>
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

struct NextExpected {
double _min, _max, minCur, maxCur;
const double epsilon = 1e-6;
NextExpected(double mn, double mx) : _min{mn}, _max{mx}, minCur{mn}, maxCur{mx - 0.5} {}
NextExpected() = default;
auto min(double x) { _min = x; minCur = x; }
auto max(double x) { _max = x; maxCur = x - 0.5; }
auto operator() (std::array<double, 3> c) {
  for (auto i : {2, 1, 0}) {
    if (c[i] < (maxCur - epsilon)) {
      if (i > 0) {
        c[i] += 1.0;
      } else { //if (i == 0) {
        c[i] += 0.5;
        if (minCur == _min) {
          minCur = _min + 0.5;
          maxCur = _max;
        } else {
          minCur = _min;
          maxCur = _max - 0.5;
        }
      }
      for (auto j = i + 1; j <= 2; ++j) {
        c[j] = minCur;
      }
      return c;
    }
  }
  if (maxCur != _max) {
    // std::cout << minCur << ", " << maxCur <<" | " << _min << ", " << _max << '\n';
    maxCur = _max;
    minCur = _min + 0.5;
    c[0] = maxCur;
    c[1] = minCur;
    c[2] = minCur;
  }
  return c;
}

auto allMax(std::array<double, 3> c) {
  for (auto i : {0, 1, 2}) {
    if (std::fabs(c[i] - _max) > epsilon ) return false;
  }
  return true;
}
};

auto cmpApprox(const std::array<double, 3> &c1, const std::array<double, 3> &c2) {
  auto epsilon = 1e-6;
  for (auto i : {0, 1, 2}) {
    if (fabs(c1[i] - c2[i]) > epsilon) return (c1[i] > c2[i]) ? 1 : -1;
  }
  return 0;
}

auto existAnchorVac(const std::array<double, 3> &t, const std::vector<std::array<double, 3>> &v) {
  auto it = std::lower_bound(begin(v), end(v), t, [](const std::array<double, 3>& a, 
                                                     const std::array<double, 3>& b){
    if (cmpApprox(a, b) < 0) return true;
    return false;
  });
  if (it != std::end(v) && cmpApprox(*it, t) == 0) return true;
  return false;
}

auto existAnchorInter(const std::array<double, 3> &t, 
                      const std::vector<std::tuple<std::array<double, 3>, bool>> &v) {
  auto temp = std::make_tuple(t, false);
  auto it = std::lower_bound(begin(v), end(v), temp, [](
    const std::tuple<std::array<double, 3>, bool>& a,
    const std::tuple<std::array<double, 3>, bool>& b) {
    if (cmpApprox(std::get<0>(a), std::get<0>(b)) < 0) return true;
    return false;
  });
  if (it != std::end(v) && cmpApprox(std::get<0>(*it), t) == 0) return true;
  return false;
}

auto clean(std::vector<std::tuple<csaransh::Coords, csaransh::Coords, bool>> &inter, std::vector<std::tuple<csaransh::Coords, bool>> &vac, const csaransh::Info &info) {
  auto thresh = info.latticeConst;// * sqrt(3) / 2;
  for (size_t i = 0; i < vac.size(); ++i) {
    if (!std::get<1>(vac[i])) continue;
    auto min = thresh + 1e-6;
    size_t minj = 0;
    for (size_t j = 0; j < inter.size(); ++j) {
      if (!std::get<2>(inter[j])) continue;
      auto dist = csaransh::calcDist(std::get<0>(vac[i]), std::get<1>(inter[j]));
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

csaransh::DefectVecT csaransh::xyz2defects(const std::string &fname, const csaransh::Info &info) {
  using std::string; using std::vector; using std::tuple;
  using std::get;

  auto latConst = info.latticeConst;
  auto o = info.origin;
  auto origin = std::array<double, 3> {{o, o, o}};
  auto obj = csaransh::AddOffset{latConst, "bcc", origin};
  /*
  std::cout << "latConst: " << latConst << '\n';
  std::cout << "origin: " << origin << '\n';
  */
  const auto nn = 0.3 * latConst; // for interstitial threshold
  vector<tuple<csaransh::Coords, double, csaransh::Coords>> uno;
  vector<tuple<csaransh::Coords, csaransh::Coords>> interThresh;
  vector<tuple<csaransh::Coords, csaransh::Coords, bool>> interClean;
  vector<tuple<csaransh::Coords, bool>> vacClean;
  csaransh::DefectVecT defects;
  uno.reserve(info.ncell * info.ncell * info.ncell * 2);
  // read file and apply object
  std::ifstream infile{fname};
  std::string line;
  bool isInsideFrame = false;
  if (infile.bad() || !infile.is_open()) return defects;
  while (std::getline(infile, line)) {
    auto first = std::find_if(begin(line), end(line), [](int ch) {
      return !std::isspace(ch);
    });
    if (first == std::end(line)) continue; // possibly blank line
    if (std::isdigit(*first)) continue;  // possibly first line with frame number
    auto second = std::find_if(first, end(line), [](int ch) {
      return std::isspace(ch);
    });
    std::string word{first, second};
    if (word == "Frame") {
      if (!isInsideFrame) {
        isInsideFrame = true;
        continue;
      } else {
        break;
      }
    }
    csaransh::Coords c;
    for (int i = 0; i < 3; ++i) {
      first = std::find_if(second, end(line), [](int ch) {
        return !std::isspace(ch);
      });
      second = std::find_if(first, end(line), [](int ch) {
        return std::isspace(ch);
      });
      c[i] = std::stod(std::string{first, second});
    }
    uno.emplace_back(obj(c));
  }
  infile.close();
  if (uno.empty()) return defects;
  std::sort(begin(uno), end(uno));
  auto lastRow = std::get<0>(uno[uno.size() - 1]);
  NextExpected nextExpected;
  //std::cout << "max: " << std::get<0>(uno[uno.size() -1])[0] << '\n';
  //std::cout << "min: " << std::get<0>(uno[0])[0] << '\n';
  nextExpected.max(std::get<0>(uno[uno.size() -1])[0]);
  nextExpected.min(std::get<0>(uno[0])[0]);
  auto expected = csaransh::Coords {{nextExpected._min, nextExpected._min, nextExpected._min}};
  std::tuple<double, csaransh::Coords, csaransh::Coords> pre;
  bool isPre = false;
  for (const auto& row : uno) {
    // threshold
    if (get<1>(row) > nn) {
      interThresh.emplace_back(get<0>(row), get<2>(row));
    }
    // clean
    csaransh::Coords ar = std::get<0>(row); double x = std::get<1>(row); csaransh::Coords c = std::get<2>(row);
    using vecB = std::vector<std::tuple<csaransh::Coords, double, csaransh::Coords>>;
    if (cmpApprox(ar, expected) == 0) {
      expected = nextExpected(expected);
      pre = std::make_tuple(x, ar, c);
      isPre = true;
    } else if (cmpApprox(ar, expected) > 0) {
      vecB res;
      while (cmpApprox(ar, expected) > 0 && cmpApprox(expected, lastRow) < 0 && !nextExpected.allMax(expected)) {
        vacClean.emplace_back(expected, true);
        //std::cout << expected << '\n';
        expected = nextExpected(expected);
      }
      expected = nextExpected(expected);
    } else {
      vecB res;
      if (isPre && std::get<1>(pre) == ar) {
        auto isPreReal = std::get<0>(pre) > x;
        interClean.emplace_back(std::get<1>(pre), std::get<2>(pre), isPreReal);
        interClean.emplace_back(ar, c, !isPreReal);
        vacClean.emplace_back(ar, false);
      } else {
        interClean.emplace_back(ar, c, true);
      }
      isPre = false;
    }
  }
  uno.clear();
  
  defects.reserve(2 * vacClean.size());
  auto vacCleanSave = vacClean;
  auto anchor2coord = [&latConst](csaransh::Coords c) { for (auto& x : c) x *= latConst; return c; };
  auto count = 1;
  for (auto &it : vacClean) {
    for (auto& x : std::get<0>(it)) x *= latConst;
  }
  clean(interClean, vacClean, info);
  //std::cout << "interstitials: \n";
  //for (auto it : interClean) std::cout << it << '\n';
  //std::cout << "vacancies: " << vacClean << '\n';
  /*
  if (interClean.size() != vacClean.size()) {
    std::cout << "Error: \n";
  }
  */
  for (const auto &it : interClean) {
    defects.emplace_back(std::move(get<1>(it)), true, count++, get<2>(it));
  }
  for (const auto &it : vacClean) {
    defects.emplace_back(std::move(get<0>(it)), false, count++, get<1>(it));
  }
  for (const auto &it : interThresh) {
    if (!existAnchorInter(std::get<0>(it), vacCleanSave)) {
      defects.emplace_back(anchor2coord(std::move(get<0>(it))), false, count++, false);
      defects.emplace_back(std::move(get<1>(it)), true, count++, false);
    }
  }
  return defects;
}

/*
void testExpected(int argc, char* argv[]) {
    using std::stod;
    NextExpected nextExpected;
    nextExpected.min(stod(argv[4]));
    nextExpected.max(stod(argv[5]));
    auto c = std::array<double, 3> {{stod(argv[1]), stod(argv[2]), stod(argv[3])}};
    auto x = nextExpected(c);
    std::cout << x << '\n';
}

void testOffset(int argc, char *argv[]) {
  using std::stod;
  auto obj = AddOffset{2.87, "bcc", std::array<double, 3>{{0.25, 0.25, 0.25}}};
  //auto obj = AddOffset{1.00, "bcc", std::array<double, 3>{{0.0, 0.0, 0.0}}};
  auto c = std::array<double, 3> {{stod(argv[1]), stod(argv[2]), stod(argv[3])}};
  std::cout << obj(c) << '\n';
}
*/
