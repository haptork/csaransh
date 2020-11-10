#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>

#include <helper.hpp>
#include <infoReader.hpp>
#include <printJson.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>

std::array<std::string, 2> separateDirAndFile(std::string path) {
  std::size_t dirPos = path.find_last_of("/");
  std::string dir{""};
  std::string file = path;
  if (dirPos != std::string::npos) {
    dir = path.substr(0, dirPos);
    file = path.substr(dirPos + 1);
  }
  return std::array<std::string, 2>{{dir, file}};
}

std::array<std::string, 2> separateFileAndExt(std::string path) {
  std::size_t extPos = path.find_last_of(".");
  std::string fname;
  if (extPos != std::string::npos)
    fname = path.substr(0, extPos);
  else
    return std::array<std::string, 2>{{path, fname}};
  std::string ext = path.substr(extPos + 1);
  return std::array<std::string, 2>{{fname, ext}};
}

std::pair<std::string, std::string>
csaransh::getInfileFromXyzfile(std::string xyzfile) {
  auto dirFile = separateDirAndFile(xyzfile);
  auto fnameExt = separateFileAndExt(dirFile[1]);
  auto addum = (dirFile[0].length() > 0) ? "/" : "";
  auto origFnameExt = dirFile[1];
  auto origFname = fnameExt[0];
  csaransh::replaceStr(dirFile[1], "fpos", "md");
  csaransh::replaceStr(fnameExt[0], "fpos", "md");
  auto filePaths = std::array<std::string, 6>{
      {dirFile[0] + addum + fnameExt[0] + ".in",
       dirFile[0] + addum + dirFile[1] + ".in",
       dirFile[0] + addum + origFnameExt + ".in",
       dirFile[0] + addum + origFname + ".in",
       dirFile[0] + addum + "common_input.in", "common_input.in"}};
  for (const auto &filePath : filePaths) {
    std::ifstream infile(filePath);
    if (!infile.bad() && infile.is_open()) {
      infile.close();
      return std::make_pair(filePath, origFnameExt);
    }
    if (infile.is_open()) infile.close();
  }
  return std::make_pair(std::string{}, origFnameExt);
}

// extract information from input file
std::tuple<csaransh::InputInfo, csaransh::ExtraInfo, bool>
csaransh::extractInfoLammps(std::string fpath, std::string ftag) {
  std::ifstream infile(fpath);
  csaransh::InputInfo mainInfo;
  csaransh::ExtraInfo extraInfo;
  if (infile.bad() || !infile.is_open()) {
    return std::make_tuple(mainInfo, extraInfo, false);
  }
  std::string line;
  int xyzrec{0};
  bool careAboutAngles = false;
  auto isOrigin = 0;
  csaransh::Coords velocity{{0.0, 0.0, 0.0}};
  while (std::getline(infile, line)) {
    auto eq =
        std::find_if(begin(line), end(line), [](int ch) { return ch == '='; });
    if (eq != std::end(line)) {
      auto cmd = trim(std::string{std::begin(line), eq});
      ++eq;
      auto val = removeParcasComments(trim(std::string{eq, std::end(line)}));
      if (cmd == "substrate") {
        extraInfo.substrate = val;
      } else if (cmd == "latticeConstant") {
        mainInfo.latticeConst = std::stod(val);
      } else if (cmd == "ncell") {
        mainInfo.ncell = std::stod(val);
      } else if (cmd == "recen") {
        extraInfo.energy = (std::stod(val)) / 1000.0;
      } else if (cmd == "xrec") {
        extraInfo.xrec = std::stod(val);
        xyzrec++;
      } else if (cmd == "yrec") {
        extraInfo.yrec = std::stod(val);
        xyzrec++;
      } else if (cmd == "zrec") {
        extraInfo.zrec = std::stod(val);
        xyzrec++;
      } else if (cmd == "vx") {
        velocity[0] = std::stod(val);
        careAboutAngles = true;
      } else if (cmd == "vy") {
        velocity[1] = std::stod(val);
        careAboutAngles = true;
      } else if (cmd == "vz") {
        velocity[2] = std::stod(val);
        careAboutAngles = true;
      } else if (cmd == "offset") {
        mainInfo.originX = std::stod(val);
        mainInfo.originY = std::stod(val);
        mainInfo.originZ = std::stod(val);
        isOrigin = 3;
      } else if (cmd == "offsetX") {
        mainInfo.originX = std::stod(val);
        isOrigin++;
      } else if (cmd == "offsetY") {
        mainInfo.originY = std::stod(val);
        isOrigin++;
      } else if (cmd == "offsetZ") {
        mainInfo.originZ = std::stod(val);
        isOrigin++;
      } else if (cmd == "temp") {
        mainInfo.temperature = std::stod(val);
      } else if (cmd == "es") {
        extraInfo.es = (val == "true");
      } else if (cmd == "offsetToUse") {
        if (val == "0") mainInfo.originType = 0;
        if (val == "1") mainInfo.originType = 1;
        if (val == "2") mainInfo.originType = 2;
      } else if (cmd == "author") {
        extraInfo.author = val;
      } else if (cmd == "potentialUsed") {
        extraInfo.potentialUsed = val;
      }
      /*
            } else if (cmd == "latConstToUse") {
              if (val == "0") mainInfo.latConstType = 0;
              if (val == "1") mainInfo.latConstType = 1;
              if (val == "2") mainInfo.latConstType = 2;
      */
    }
  }
  infile.close();
  if (isOrigin < 3) mainInfo.originType = 1;
  extraInfo.isPkaGiven = (xyzrec >= 3) ? true : false;
  if (mainInfo.latticeConst < 0.0) {
    return std::make_tuple(mainInfo, extraInfo, false);
  }
  mainInfo.structure = "bcc"; // TODO: extend for fcc, no assumption
  extraInfo.infile = fpath;
  extraInfo.rectheta =
      (careAboutAngles) ? std::atan(velocity[1] / velocity[0]) : 0.0;
  extraInfo.recphi =
      (careAboutAngles) ? std::atan(velocity[2] / velocity[0]) : 0.0;
  return std::make_tuple(mainInfo, extraInfo, true);
}

// extract information from input file
std::tuple<csaransh::InputInfo, csaransh::ExtraInfo, bool>
csaransh::extractInfoParcas(std::string fpath, std::string ftag) {
  std::ifstream infile(fpath);
  csaransh::InputInfo mainInfo;
  csaransh::ExtraInfo extraInfo;
  if (!infile.bad() && infile.is_open()) {
    std::string line;
    auto count = 0;
    while (std::getline(infile, line)) {
      auto i = line.find('=');
      if (i == 9) {
        auto cmd = trim(line.substr(0, 9));
        if (cmd == "substrate") {
          extraInfo.substrate = removeParcasComments(line.substr(10));
          count++;
        } else if (cmd == "box(1)") {
          mainInfo.boxSize = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "ncell(1)") {
          mainInfo.ncell = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "recen") {
          extraInfo.energy =
              (std::stod(removeParcasComments(line.substr(10)))) / 1000.0;
          count++;
        } else if (cmd == "xrec") {
          extraInfo.xrec = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "yrec") {
          extraInfo.yrec = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "zrec") {
          extraInfo.zrec = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "rectheta") {
          extraInfo.rectheta = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "recphi") {
          extraInfo.recphi = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "offset(1)") {
          mainInfo.originX = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "offset(2)") {
          mainInfo.originY = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "offset(3)") {
          mainInfo.originZ = std::stod(removeParcasComments(line.substr(10)));
          count++;
        } else if (cmd == "temp") {
          mainInfo.temperature =
              std::stod(removeParcasComments(line.substr(10)));
          count++;
        }
      }
    }
    mainInfo.structure = "bcc"; // TODO: extend for fcc, without assumption
    infile.close();
    if (count == 13) { // got all the info
      mainInfo.latticeConst = mainInfo.boxSize / mainInfo.ncell;
      extraInfo.isPkaGiven = true;
      extraInfo.infile = fpath;
      return std::make_tuple(mainInfo, extraInfo, true);
    }
    return std::make_tuple(mainInfo, extraInfo, false);
  }
  return std::make_tuple(mainInfo, extraInfo, false);
}

std::pair<csaransh::XyzFileType, bool>
csaransh::getSimulationCode(std::string fname) {
  std::ifstream infile(fname);
  if (!infile.bad() && infile.is_open()) {
    std::string line;
    auto lineNo = 0;
    constexpr auto maxLinesToLook = 10;
    std::vector<std::string> keyWords{"CASCADESDBLIKECOLS", "PARCAS",
                                      "LAMMPS-XYZ", "LAMMPS-DISP"};
    std::vector<csaransh::XyzFileType> codes{
        csaransh::XyzFileType::cascadesDbLikeCols,
        csaransh::XyzFileType::parcasWithStdHeader,
        csaransh::XyzFileType::lammpsWithStdHeader,
        csaransh::XyzFileType::lammpsDisplacedCompute};
    while (std::getline(infile, line) && lineNo++ < maxLinesToLook) {
      for (size_t i = 0; i < keyWords.size(); i++) {
        auto pos = line.find(keyWords[i]);
        if (pos != std::string::npos) return std::make_pair(codes[i], true);
      }
    }
  }
  return std::make_pair(csaransh::XyzFileType{}, false);
}