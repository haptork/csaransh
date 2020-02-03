#include <string>

#include <helper.hpp>

std::string csaransh::errToStr(csaransh::ErrorStatus err) {
  if (err == csaransh::ErrorStatus::inputFileMissing) {
    return "Could not read input file; You can make a common_input.in file in "
           "current dir.";
  } else if (err == csaransh::ErrorStatus::inputFileMissing) {
    return "Could not read input file; You can make a common_input.in file in "
           "current dir.";
  } else if (err == csaransh::ErrorStatus::InputFileincomplete) {
    return "Input file doesn't have all the info; Refer the sample input files "
           "in examples.";
  } else if (err == csaransh::ErrorStatus::unknownSimulator) {
    return "Input file doesn't have LAMMPS/PARCAS/DISPLACED simulation input "
           "type; Refer the sample input files in examples.";
  } else if (err == csaransh::ErrorStatus::xyzFileDefectsProcessingError) {
    return "XYZ file has too many defects or zero atoms";
  }
  return "Unknown Error.";
}
