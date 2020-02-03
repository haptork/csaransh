#include <algorithm>

#include <AddOffset.hpp>
#include <optimizeLatticeConst.hpp>

auto optimizeForOffset(std::vector<csaransh::Coords> &atoms, double minLatConst,
                       double maxLatConst, double step) {
  double resLatConst = 0.0;
  double minOffsetTotal = std::numeric_limits<double>::max();
  std::vector<double> allOffsets;
  allOffsets.resize(atoms.size());
  for (double curLatConst = minLatConst; curLatConst < maxLatConst;
       curLatConst += step) {
    double res = 0.0;
    csaransh::AddOffset addOffset{curLatConst, "bcc",
                                  csaransh::Coords{{0., 0., 0.}}};
    for (size_t i = 0; i < atoms.size(); i++) {
      for (auto c : atoms[i]) {
        auto offset = std::fmod(c, curLatConst / 4.0);
        // auto offset = std::fmod(c, curLatConst / 2.0);
        // if (offset > curLatConst / 4.0) offset -= curLatConst/2.0;
        allOffsets[i] = offset;
      }
    }
    auto toConsider = allOffsets.size() - 1000;
    std::nth_element(
        begin(allOffsets), begin(allOffsets) + toConsider, end(allOffsets),
        [](double a, double b) { return std::fabs(a) < std::fabs(b); });
    for (size_t i = 0; i < toConsider; i++) {
      res += allOffsets[i];
    }
    if (res < minOffsetTotal) {
      minOffsetTotal = res;
      resLatConst = curLatConst;
    }
    // std::cout << curLatConst << " " << res / toConsider << '\n';
  }
  return resLatConst;
}

auto optimizeLatConst(std::vector<csaransh::Coords> &atoms, double latConst) {
  double minLatConst = latConst - 0.015;
  double maxLatConst = latConst + 0.015;
  double step = 0.001;
  auto resLatConst = optimizeForOffset(atoms, minLatConst, maxLatConst, step);
  minLatConst = resLatConst - step;
  maxLatConst = resLatConst + step;
  step /= 10;
  resLatConst = optimizeForOffset(atoms, minLatConst, maxLatConst, step);
  return resLatConst;
}