#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <cluster2features.hpp>
#include <helper.hpp>
#include <reader.hpp>

/*
*/

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "No input xyz file provided as cmd argument.\n";
    return 1;
  }
  auto src = std::string{};
  const std::string outpath{"./cascades-data.json"};
  std::ofstream outfile{outpath};
  if (!outfile.is_open()) {
    std::cerr << "The output path " + outpath + " is not accessible.\n";
    return 1;
  }
  outfile << "{\"source\": \"" << src << "\", \"data\": [\n";
  std::cout << "Total files to process: " << (argc - 1) << '\n' << std::flush;
  auto success = 0;
  for (int i = 1; i < argc; ++i) {
    std::cout << "\rCurrently processing file " << i << std::flush;
    csaransh::readStatus ret;
    ret = csaransh::processFile(argv[i], outfile, i);
    if (csaransh::readStatus::fail == ret) {
      std::cerr << "\nError in processing file " + std::string{argv[i]} + "\n";
    } else {
      ++success;
      if (i != argc - 1) outfile << ",";
      outfile << "\n";
    }
  }
  outfile << "]}"
          << "\n";
  outfile.close();
  std::cout << '\r' << success << " out of " << argc - 1
            << " processed successfully.\n";
  return 0;
}
