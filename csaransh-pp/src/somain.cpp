#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <helper.hpp>
#include <logger.hpp>
#include <printJson.hpp>
#include <reader.hpp>
#include <results.hpp>

struct InfoPyInput {
  int ncell;
  double boxSize;
  double latticeConst;
  double originX;
  double originY;
  double originZ;
  int originType;
  double temperature;
  const char *xyzFileType;
  const char *xyzFilePath;
  const char *structure;
};
struct InfoPyExtraInput {
  double energy;
  double simulationTime;
  int id{1};
  // for distribution around PKA
  bool isPkaGiven;
  double xrec{0.0};
  double yrec{0.0};
  double zrec{0.0};
  double rectheta{0.0};
  double recphi{0.0};
  // extra
  const char *substrate;
  const char *infile;
  const char *tags;
  const char *potentialUsed;
  const char *author;
};

struct PyConfig {
  bool onlyDefects{false};
  bool isFindDistribAroundPKA{true};
  bool isFindClusterFeatures{true};
  bool filterZeroSizeClusters{false};
  bool isIgnoreBoundaryDefects{false};
  bool isAddThresholdInterstitials{true};
  bool safeRunChecks{true};
  double thresholdFactor{0.345};
  double extraDefectsSafetyFactor{50.0};
  int logMode{csaransh::LogMode::warning | csaransh::LogMode::error};
  const char *logFilePath;
  const char *outputJSONFilePath;
};

auto pyConfigToCppConfig(const PyConfig &pyConfig) {
  auto config = csaransh::Config{};
  config.onlyDefects = pyConfig.onlyDefects;
  config.isFindClusterFeatures = pyConfig.isFindClusterFeatures;
  config.isFindDistribAroundPKA = pyConfig.isFindDistribAroundPKA;
  config.filterZeroSizeClusters = pyConfig.filterZeroSizeClusters;
  config.isIgnoreBoundaryDefects = pyConfig.isIgnoreBoundaryDefects;
  config.isAddThresholdInterstitials = pyConfig.isAddThresholdInterstitials;
  config.safeRunChecks = pyConfig.safeRunChecks;
  config.thresholdFactor = pyConfig.thresholdFactor;
  config.extraDefectsSafetyFactor = pyConfig.extraDefectsSafetyFactor;
  config.logMode = pyConfig.logMode;
  config.logFilePath = std::string{pyConfig.logFilePath};
  config.outputJSONFilePath = std::string{pyConfig.outputJSONFilePath};
  return config;
}

auto pyInfoToCppInfo(const InfoPyInput &pyinput,
                     const InfoPyExtraInput &pyextra) {
  csaransh::InputInfo input;
  input.ncell = pyinput.ncell;
  input.boxSize = pyinput.boxSize;
  input.latticeConst = pyinput.latticeConst;
  input.originX = pyinput.originX;
  input.originY = pyinput.originY;
  input.originZ = pyinput.originZ;
  input.originType = pyinput.originType;
  input.temperature = pyinput.temperature;
  input.xyzFilePath = std::string{pyinput.xyzFilePath};
  input.structure = std::string{pyinput.structure};
  std::string simCodeStr = std::string{pyinput.xyzFileType};
  std::transform(simCodeStr.begin(), simCodeStr.end(), simCodeStr.begin(),
                 [](unsigned char c) { return std::toupper(c); } // correct
  );
  std::vector<std::string> keyWords{"CASCADESDBLIKECOLS", "PARCAS",
                                    "LAMMPS-XYZ", "LAMMPS-DISP"};
  std::vector<csaransh::XyzFileType> codes{
      csaransh::XyzFileType::cascadesDbLikeCols,
      csaransh::XyzFileType::parcasWithStdHeader,
      csaransh::XyzFileType::lammpsWithStdHeader,
      csaransh::XyzFileType::lammpsDisplacedCompute};
  for (size_t i = 0; i < keyWords.size(); i++) {
    if (simCodeStr == keyWords[i]) {
      input.xyzFileType = codes[i];
      break;
    }
  }
  csaransh::ExtraInfo extra;
  extra.energy = pyextra.energy;
  extra.simulationTime = pyextra.simulationTime;
  extra.id = pyextra.id;
  extra.isPkaGiven = pyextra.isPkaGiven;
  extra.xrec = pyextra.xrec;
  extra.yrec = pyextra.yrec;
  extra.zrec = pyextra.zrec;
  extra.rectheta = pyextra.rectheta;
  extra.recphi = pyextra.recphi;
  extra.substrate = std::string{pyextra.substrate};
  extra.infile = std::string{pyextra.infile};
  extra.tags = std::string{pyextra.tags};
  extra.potentialUsed = std::string{pyextra.potentialUsed};
  extra.author = std::string{pyextra.author};
  return std::make_tuple(true, input, extra);
}

extern "C" char *pyProcessFile(InfoPyInput pyInfo, InfoPyExtraInput pyExtraInfo,
                               PyConfig pyConfig) {
  using csaransh::Logger;
  auto config = pyConfigToCppConfig(pyConfig);
  auto info = csaransh::InputInfo{};
  auto extraInfo = csaransh::ExtraInfo{};
  auto isSuccess = false;
  std::tie(isSuccess, info, extraInfo) = pyInfoToCppInfo(pyInfo, pyExtraInfo);
  Logger::inst().mode(config.logMode);
  Logger::inst().file(config.logFilePath);
  csaransh::resultsT res;
  if (!isSuccess) {
    res.err = csaransh::ErrorStatus::unknownSimulator;
  } else {
    Logger::inst().log_info("Started Processing file \"" + info.xyzFilePath +
                            " (" + extraInfo.infile + ") " + "\"");
    res = csaransh::process(info, extraInfo, config);
    Logger::inst().log_info("Finished Processing");
  }
  std::stringstream outfile;
  outfile << "{";
  csaransh::infoToKeyValue(outfile, info, extraInfo);
  outfile << ",";
  resToKeyValue(outfile, res);
  outfile << "}\n";
  std::string str = outfile.str();
  char *writable = (char *)malloc(
      sizeof(char) * (str.size() + 1)); // new char[str.size() + 1];
  std::copy(str.begin(), str.end(), writable);
  writable[str.size()] = '\0';
  return writable;
}

extern "C" char *pyProcessFileWoInfo(char *xyzfile, PyConfig pyConfig) {
  using csaransh::Logger;
  auto config = pyConfigToCppConfig(pyConfig);
  auto xyzfileStr = std::string(xyzfile);
  Logger::inst().mode(config.logMode);
  Logger::inst().file(config.logFilePath);
  Logger::inst().log_info("Started Processing file \"" + xyzfileStr + "\"");
  std::stringstream outfile;
  auto res = csaransh::processFile(xyzfileStr, outfile, config, 0);
  Logger::inst().log_info("Finished Processing");
  std::string str = outfile.str();
  char *writable = (char *)malloc(
      sizeof(char) * (str.size() + 1)); // new char[str.size() + 1];
  std::copy(str.begin(), str.end(), writable);
  writable[str.size()] = '\0';
  return writable;
}

extern "C" void dalloc(void *x) { free(x); }