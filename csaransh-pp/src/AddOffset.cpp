/*!
 * @file
 * src for AddOffset class.
 * */
#include "AddOffset.hpp"
#include <cmath>
#include <iostream>

bool csaransh::AddOffset::_isUnitcell(double x, double y, double z, double l,
                            std::array<double, 3> origin) {
  double pos[3] = {x, y, z};
  double epsilon = 1e-6;
  for (int i = 0; i < 3; i++) {
    if (!(pos[i] >= (origin[i] * l - epsilon) && 
          pos[i] <= (origin[i] * l + l + epsilon)))
      return false;
  }
  return true;
}

/*
 * The bcc lattice can be seen as formed of two simple lattices interwoven.
 * The only positions we want to know are origins of these two simple lattices.
 * First one is at the origin of the bcc lattice and the other one is the body
 * centered atom of the first unitcell.
 * */
void csaransh::AddOffset::_bccUnitcell() {
  /*
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        if (_isUnitcell(i * lc, j * lc, k * lc, lc, origin)) {
          std::array<double, 3> p1;
          p1[0] = (i - origin[0]) * lc, p1[1] = (j - origin[1]) * lc,
          p1[2] = (k - origin[2]) * lc;
          v.push_back(std::move(p1));
        }
        if (v.size() == 2) return;
        if (_isUnitcell((i + 0.5) * lc, (j + 0.5) * lc, (k + 0.5) * lc, lc,
                        origin)) {
          std::array<double, 3> p1;
          p1[0] = (i + 0.5 - origin[0]) * lc,
          p1[1] = (j + 0.5 - origin[1]) * lc,
          p1[2] = (k + 0.5 - origin[2]) * lc;
          v.push_back(std::move(p1));
        }
        if (v.size() == 2) return;
      }
    }
  }
  */
  _sites.clear();
  _sites.emplace_back(std::array<long double, 3>{{0.0, 0.0, 0.0}});
  _sites.emplace_back(std::array<long double, 3>{{0.5, 0.5, 0.5}});
  //_sites.emplace_back(std::array<double, 3>{{-0.5, -0.5, -0.5}});
  for(auto& it : _sites) {
    //auto i = 0;
    for (auto& jt : it) {
      //jt += origin[i];
      //jt = fmodf(jt, 1.0);
      jt *= _latConst;//lc;
    }
  }

}

/*
 * The fcc lattice can be seen as formed of four simple lattices interwoven.
 * The only positions we want to know are origins of these four simple lattices.
 * Since, here we are not computing these four coordinates first we can't return
 * after first four like we do in bcc. The complete unitcell is thus added.
 * TODO: Test thoroughly
 * */
void csaransh::AddOffset::_fccUnitcell() {
  /*
  for (int i = -1; i < 3; i++) {
    for (int j = -1; j < 3; j++) {
      for (int k = -1; k < 3; k++) {
        if (_isUnitcell(i * lc, j * lc, k * lc, lc, origin)) {
          std::array<double, 3> p1;
          p1[0] = (i - origin[0]) * lc, p1[1] = (j - origin[1]) * lc,
          p1[2] = (k - origin[2]) * lc;
          v.push_back(std::move(p1));
        }
        if (_isUnitcell(i * lc, (j + 0.5) * lc, (k + 0.5) * lc, lc, origin)) {
          std::array<double, 3> p1;
          p1[0] = (i - origin[0]) * lc, p1[1] = (j + 0.5 - origin[1]) * lc,
          p1[2] = (k + 0.5 - origin[2]) * lc;
          v.push_back(std::move(p1));
        }
        if (_isUnitcell((i + 0.5) * lc, j * lc, (k + 0.5) * lc, lc, origin)) {
          std::array<double, 3> p1;
          p1[0] = (i + 0.5 - origin[0]) * lc, p1[1] = (j - origin[1]) * lc,
          p1[2] = (k + 0.5 - origin[2]) * lc;
          v.push_back(std::move(p1));
        }
        if (_isUnitcell((i + 0.5) * lc, (j + 0.5) * lc, k * lc, lc, origin)) {
          std::array<double, 3> p1;
          p1[0] = (i + 0.5 - origin[0]) * lc,
          p1[1] = (j + 0.5 - origin[1]) * lc, p1[2] = (k - origin[2]) * lc;
          v.push_back(std::move(p1));
        }
      }
    }
  }
  */
  _sites.clear();
  _sites.emplace_back(std::array<long double, 3>{{0.0, 0.0, 0.0}});
  _sites.emplace_back(std::array<long double, 3>{{0.5, 0.5, 0.0}});
  _sites.emplace_back(std::array<long double, 3>{{0.5, 0.0, 0.5}});
  _sites.emplace_back(std::array<long double, 3>{{0.0, 0.5, 0.5}});
  for(auto& it : _sites) {
    for (auto& jt : it) {
      jt *= _latConst;
    }
  }
}

/**
 * calculate shorter of the distance between two points considering a periodic
 * boundary of a given size. The distance shorter of the two distances (direct
 * and mirror / through periodic boundary) is considered and information about
 * this is stored in the output parameter mirror.
 **/
double csaransh::AddOffset::_calcDistMirror(std::array<long double, 3> a, std::array<long double, 3> b,
                                 long double size, std::array<int, 3> &mirror) {
  long double res = 0;
  for (int i = 0; i < 3; i++) {
    long double dist = fabs(a[i] - b[i]);
    if (dist > size * 0.5) {
      dist = size - dist;
      mirror[i] = b[i] < 0 ? -1 : 1;
    } else {
      mirror[i] = 0;
    }
    res += dist * dist;
  }
  return std::sqrt(res);
}

csaransh::AddOffset::AddOffset(double latConst, std::string lattice,
                     std::array<double, 3> origin) {
  _origin = std::array<long double, 3> {{origin[0], origin[1], origin[2]}};
  _latConst = latConst;
  if (lattice[0] == 'b') {
    _bccUnitcell();
  } else if (lattice[0] == 'f') {
    _fccUnitcell();
  } else {
    // TODO log error
  }
}

template <class T>
void print(std::array<T, 3> x) {
  for (auto jt : x) {
    std::cout << jt << ", ";
  }
  std::cout << '\n';
}

//constexpr bool debug = false;

std::tuple<std::array<double,3>, double, std::array<double, 3>>
csaransh::AddOffset::operator()(const std::array<double, 3> &c) {
  std::array<long double, 3> coords {{c[0], c[1], c[2]}};
  std::array<long double, 3> modCoords;
  std::array<long double, 3> divCoords;
  std::array<double, 3> cellPos;
  for (int i = 0; i < 3; i++) {
    long double orig = coords[i] - (_origin[i]*_latConst);
    modCoords[i] = std::fmod(orig, _latConst);
    divCoords[i] = int(orig / _latConst);
  }
/*
  if (debug) {
    for (auto it : _sites) print(it);
    std::cout << "modCoords: "; print(modCoords);
    std::cout << "divCoords: "; print(divCoords);
  }
  */
  auto min = -1.;
  //auto i = 0;
  std::array<int, 3> mirror;
  for (auto it : _sites) {
    auto temp = _calcDistMirror(it, modCoords, _latConst, mirror);
    /*
    if (debug) {
      std::cout <<'\n';
      print(it);
      std::cout << "mirror: "; print(mirror);
      std::cout << "temp: " << temp << ": ";
      std::cout <<'\n';
    }
    */
    if (temp < min || min == -1) {
      min = temp;
      for (int i = 0; i < 3; i++) {
        cellPos[i] = (divCoords[i] + mirror[i] + it[i] / _latConst) + _origin[i];
        cellPos[i] = floorf(cellPos[i] * 100) / 100;
      }
    }
    //i++;
  }
  /*
  if (debug) {
    std::cout << "nearest lattice site: "; 
    print(asdf);
  }
  */
  return std::make_tuple(cellPos, min, c);
}
