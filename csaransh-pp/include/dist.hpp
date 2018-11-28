#include <array>
#include <tuple>


double calcDist (std::array<double, 3> a, std::array<double, 3> b);

// given two 3d points tells if they are close than a threshold
class AreClose {
public:
  using AreClsType = std::tuple<int, std::array<double, 3>, bool>;
  AreClose(double t) : _threshold{t} {}

  bool operator() (AreClsType a, AreClsType b) {
    using std::get;
    return (calcDist(get<1>(a), get<1>(b)) < _threshold);  
  }

private:
  double _threshold;
};

class AreClose2 {
public:
  using AreClsType = std::tuple<int, std::array<double, 3>, int, bool>;
  AreClose2(double t, double t2) : _threshold{t}, _threshold2{t2} {}

  bool operator() (AreClsType a, AreClsType b) {
    using std::get;
    if (get<2>(a) || get<2>(b)) {
      return (calcDist(get<1>(a), get<1>(b)) < _threshold);  
    }
    return (calcDist(get<1>(a), get<1>(b)) < _threshold2);  
  }

private:
  double _threshold;
  double _threshold2;
};




