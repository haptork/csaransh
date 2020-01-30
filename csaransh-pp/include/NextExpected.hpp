/*!
 * @file
 * helper functions for general usage
 * */

#ifndef NEXTEXPECTED_CSARANSH_HPP
#define NEXTEXPECTED_CSARANSH_HPP

#include <helper.hpp>

namespace csaransh {
struct NextExpected {
private:
  Coords _min, _max, _minCur, _maxCur;
  Coords _cur;

public:
  NextExpected(Coords mn, Coords mx, Coords maxCur)
      : _min{mn}, _max{mx}, _cur{mn}, _minCur{mn}, _maxCur{maxCur} {}

  NextExpected() = default;
  auto min() const { return _min; }
  auto max() const { return _max; }
  auto minCur() const { return _minCur; }
  auto maxCur() const { return _maxCur; }
  const Coords &cur() const { return _cur; }
  bool allMax() const {
    constexpr auto epsilon = 1e-4; // std::numeric_limits<double>::epsilon();
    for (size_t i = 0; i < _cur.size(); i++) {
      if ((_cur[i] + epsilon) < _max[i]) return false;
    }
    return true;
  }
  const Coords &increment();
};
} // namespace csaransh
#endif
