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
  int xyzColumnStart;
  const char *xyzFileType;
  const char *xyzFilePath;
  const char *structure;
};
struct InfoPyExtraInput {
  double energy;
  double simulationTime;
  // for distribution around PKA
  bool isPkaGiven;
  bool es;
  double xrec{0.0};
  double yrec{0.0};
  double zrec{0.0};
  double rectheta{0.0};
  double recphi{0.0};
  // extra
  const char *id;
  const char *substrate;
  const char *infile;
  const char *tags;
  const char *potentialUsed;
  const char *author;
};

struct PyConfig {
  bool allFrames{false};
  bool onlyDefects{false};
  bool isFindDistribAroundPKA{true};
  bool isFindClusterFeatures{true};
  bool filterZeroSizeClusters{false};
  bool isIgnoreBoundaryDefects{false};
  bool isAddThresholdDefects{true};
  bool safeRunChecks{true};
  double thresholdFactor{0.345};
  double extraDefectsSafetyFactor{50.0};
  int logMode{csaransh::LogMode::warning | csaransh::LogMode::error};
  const char *logFilePath;
  const char *outputJSONFilePath;
};

auto pyConfigToCppConfig(const PyConfig &pyConfig) {
  auto config = csaransh::Config{};
  config.allFrames = pyConfig.allFrames;
  config.onlyDefects = pyConfig.onlyDefects;
  config.isFindClusterFeatures = pyConfig.isFindClusterFeatures;
  config.isFindDistribAroundPKA = pyConfig.isFindDistribAroundPKA;
  config.filterZeroSizeClusters = pyConfig.filterZeroSizeClusters;
  config.isIgnoreBoundaryDefects = pyConfig.isIgnoreBoundaryDefects;
  config.isAddThresholdDefects = pyConfig.isAddThresholdDefects;
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
  input.xyzColumnStart = pyinput.xyzColumnStart;
  input.xyzFilePath = std::string{pyinput.xyzFilePath};
  input.structure = std::string{pyinput.structure};
  std::string simCodeStr = std::string{pyinput.xyzFileType};
  std::transform(simCodeStr.begin(), simCodeStr.end(), simCodeStr.begin(),
                 [](unsigned char c) { return std::toupper(c); } // correct
  );
  std::vector<std::string> keyWords{"GENERIC", "CASCADESDBLIKECOLS", "PARCAS",
                                    "LAMMPS-XYZ", "LAMMPS-DISP"};
  std::vector<csaransh::XyzFileType> codes{
      csaransh::XyzFileType::generic,
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
  extra.isPkaGiven = pyextra.isPkaGiven;
  extra.es = pyextra.es;
  extra.xrec = pyextra.xrec;
  extra.yrec = pyextra.yrec;
  extra.zrec = pyextra.zrec;
  extra.rectheta = pyextra.rectheta;
  extra.recphi = pyextra.recphi;
  extra.id = std::string{pyextra.id};
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
  std::stringstream outfile;
  auto res = csaransh::resultsT{};
  auto success = 0;
  if (!isSuccess) {
    res.err = csaransh::ErrorStatus::unknownSimulator;
  } else {
    Logger::inst().log_info("Started Processing file \"" + info.xyzFilePath +
                            " (" + extraInfo.infile + ") " + "\"");
    csaransh::frameStatus fs = csaransh::frameStatus::prelude;
    std::ifstream xyzfile{info.xyzFilePath};
    if (xyzfile.bad() || !xyzfile.is_open()) {
      res.err = csaransh::ErrorStatus::xyzFileReadError;
    }
    auto frameCount = 0;
    auto initId = extraInfo.id;
    auto origSimTime = extraInfo.simulationTime;
    while (true) {
      if (config.allFrames && origSimTime == 0) extraInfo.simulationTime = success + 1;
      if (config.allFrames) extraInfo.id = initId + "_" + std::to_string(success + 1);
      auto res = csaransh::processTimeFile(info, extraInfo, config, xyzfile, fs, outfile, success == 0);
      frameCount++;
      if (res.second != csaransh::ErrorStatus::noError) {
        Logger::inst().log_info("Error processing" + std::to_string(frameCount) +" frame in file \"" + info.xyzFilePath + "\"");
      } else {
        ++success;
        if (config.allFrames) Logger::inst().log_info("Finished processing" + std::to_string(success) +" frame in file \"" + info.xyzFilePath + "\"");
      }
      if (res.first == csaransh::xyzFileStatus::eof) break;
    }
    xyzfile.close();
    Logger::inst().log_info("Finished Processing");
  }
  if (success == 0) csaransh::printJson(outfile, info, extraInfo, res);
  std::string str = outfile.str();
  char *writable = (char *)malloc(sizeof(char) * (str.size() + 1));
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
  auto res = csaransh::processFileTimeCmd(xyzfileStr, outfile, config, 0);
  Logger::inst().log_info("Finished Processing");
  std::string str = outfile.str();
  char *writable = (char *)malloc(
      sizeof(char) * (str.size() + 1)); // new char[str.size() + 1];
  std::copy(str.begin(), str.end(), writable);
  writable[str.size()] = '\0';
  return writable;
}

extern "C" void dalloc(void *x) { free(x); }