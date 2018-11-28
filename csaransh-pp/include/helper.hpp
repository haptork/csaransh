#ifndef AIE_HELPER
#define AIE_HELPER

struct Info {
  std::string infile;
  std::string name;
  std::string substrate;
  double boxSize;
  double energy;
  int ncell;
  double rectheta;
  double recphi;
  double xrec;
  double yrec;
  double zrec;
  double latticeConst;
  double origin;
};

#endif