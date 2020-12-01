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

extern "C" int pyProcessFileTime(InfoPyInput pyInfo, InfoPyExtraInput pyExtraInfo,
                               PyConfig pyConfig) {
  using csaransh::Logger;
  auto config = pyConfigToCppConfig(pyConfig);
  auto info = csaransh::InputInfo{};
  auto extraInfo = csaransh::ExtraInfo{};
  auto isSuccess = false;
  std::tie(isSuccess, info, extraInfo) = pyInfoToCppInfo(pyInfo, pyExtraInfo);
  auto res = csaransh::resultsT{};
  if (!isSuccess) {
    res.err = csaransh::ErrorStatus::unknownSimulator;
    return 1;
  }
  Logger::inst().mode(config.logMode);
  Logger::inst().file(config.logFilePath);
  std::ifstream infile{info.xyzFilePath};
  if (infile.bad() || !infile.is_open()) return 1;
  const std::string outpath{config.outputJSONFilePath};
  std::ofstream outfile{outpath};
  if (!outfile.is_open()) {
    std::cerr << "The output path " + outpath + " is not accessible.\n";
    return 1;
  }
  outfile << "{\"meta\": {\n";
  csaransh::configToKeyValue(outfile, config);
  outfile << "}\n, \"data\": [\n";
  Logger::inst().log_info("Started writing to output file \"" + outpath + "\"");
  Logger::inst().log_info("Started Processing file \"" + info.xyzFilePath +
                          " (" + extraInfo.infile + ") " + "\"");
  csaransh::frameStatus fs = csaransh::frameStatus::prelude;
  auto success = 0;
  int curIndex = 0;
  auto initId = extraInfo.id;
  while (true) {
    extraInfo.simulationTime = success + 1;
    if (config.allFrames) extraInfo.id = initId + "_" + std::to_string(success);
    auto ret = csaransh::processTimeFile(info, extraInfo, config, infile, fs, outfile);
    if (ret.second != csaransh::ErrorStatus::noError) {
      std::cerr << "\nError in processing file at " << curIndex << '\n';
      std::cerr << errToStr(ret.second) << '\n';
      Logger::inst().log_error("Error in processing file at \"" + std::to_string(curIndex) + "\" " + errToStr(ret.second));
    } else {
      ++success;
    }
    if (ret.first == csaransh::xyzFileStatus::eof) {
      Logger::inst().log_info("Finished processing file at \"" + std::to_string(curIndex) + "\"");
      break;
    }
    curIndex++;
  }
  infile.close();
  Logger::inst().log_info("Finished Processing");
  outfile << "]}"
          << "\n";
  outfile.close();
  Logger::inst().log_info("Output file written " + outpath);
  return success;
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