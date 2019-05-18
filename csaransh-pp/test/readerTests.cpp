#include <catch.hpp>

#include <reader.hpp>

using namespace csaransh;
using namespace std::string_literals;

SCENARIO("The user only needs to give xyz file for all the atoms or displaced "
         "atoms, the sofware should look for the input file with the similar "
         "name and fixed extension '.in' in the same directory, the special "
         "tag fpos in the xyz file name should be replaced by md in the input "
         "file once (and not in the path) (checks getInfileFromXyzfile, "
         "separateDirAndFile, separateFileAndExt",
         "[reader]") {
  SECTION("Normal cases") {
    auto x = getInfileFromXyzfile("yellow.xyz");
    REQUIRE(getInfileFromXyzfile("yellow.xyz") ==
            std::make_pair("yellow.in"s, "yellow"s));
    REQUIRE(getInfileFromXyzfile("path/to/yellow.xyz") ==
            std::make_pair("path/to/yellow.in"s, "yellow"s));
    REQUIRE(getInfileFromXyzfile("/path/to/yellow.disp") ==
            std::make_pair("/path/to/yellow.in"s, "yellow"s));
    REQUIRE(getInfileFromXyzfile("./path/to/yellow.whatever") ==
            std::make_pair("./path/to/yellow.in"s, "yellow"s));
    REQUIRE(getInfileFromXyzfile("../path/to/yellow.xyz.whatever") ==
            std::make_pair("../path/to/yellow.xyz.in"s, "yellow.xyz"s));
  }
  SECTION("Specific to naming convention used for parcas in IAEA contest, if "
          "the file name contains fpos, it will be replaced by md") {
    REQUIRE(getInfileFromXyzfile("yellow_fpos.xyz") ==
            std::make_pair("yellow_md.in"s, "yellow_md"s));
    REQUIRE(getInfileFromXyzfile("yellowfposred.xyz") ==
            std::make_pair("yellowmdred.in"s, "yellowmdred"s));
    REQUIRE(getInfileFromXyzfile("path/to/yellow.fpos.xyz.whatever") ==
            std::make_pair("path/to/yellow.md.xyz.in"s, "yellow.md.xyz"s));
    REQUIRE(getInfileFromXyzfile("path/fpos/yellow.xyz.whatever") ==
            std::make_pair("path/fpos/yellow.xyz.in"s, "yellow.xyz"s));
    REQUIRE(getInfileFromXyzfile("/path/fpos/yellowfpos.xyz") ==
            std::make_pair("/path/fpos/yellowmd.in"s, "yellowmd"s));
    REQUIRE(getInfileFromXyzfile("/path/fpos/yellowfpos_yellowfpos.xyz") ==
            std::make_pair("/path/fpos/yellowmd_yellowfpos.in"s,
                           "yellowmd_yellowfpos"s));
  }
  SECTION("Edge cases") {
    REQUIRE(getInfileFromXyzfile("what") ==
            std::make_pair("what.in"s, "what"s));
    REQUIRE(getInfileFromXyzfile("what.") ==
            std::make_pair("what.in"s, "what"s));
  }
  SECTION("Edge cases - almost invalid inputs") { // might add warning to these
    REQUIRE(getInfileFromXyzfile("") == std::make_pair(".in"s, ""s));
    REQUIRE(getInfileFromXyzfile(".what") == std::make_pair(".in"s, ""s));
  }
}

SCENARIO("The simulation code type should be detected from the input file tag "
         "in first few lines (10 lines), the tags are: 'PARCAS', 'LAMMPS-XYZ', "
         "LAMMPS-DISP' all in capitals and as is. If no valid tags are present "
         "then it should be notified.",
         "[reader]") {
  SECTION("Positive cases: one of the simulation code tag is found") {
    REQUIRE(getSimulationCode("./data/parcas/005-md-10-1.in") ==
            std::make_pair(csaransh::SimulationCode::parcas, true));
    REQUIRE(getSimulationCode("data/lammps/Pos1.in") ==
            std::make_pair(csaransh::SimulationCode::lammps, true));
    REQUIRE(
        getSimulationCode("./data/disp/Pos2.in") ==
        std::make_pair(csaransh::SimulationCode::lammpsDisplacedCompute, true));
  }
  SECTION("Negative cases: no simulation code tag is found") {
    REQUIRE(getSimulationCode("./file/doesnt/exist") ==
            std::make_pair(csaransh::SimulationCode{}, false));
    REQUIRE(getSimulationCode("test/readerTest.cpp") ==
            std::make_pair(csaransh::SimulationCode{},
                           false)); // exists but not tag in top 10 lines
    REQUIRE(getSimulationCode("test/catchUnitTests.cpp") ==
            std::make_pair(csaransh::SimulationCode{},
                           false)); // exists but no tag at all
  }
}

/*
bool operator==(const Info& lhs, const Info& rhs) {
}
*/

SCENARIO("The software should read required simulation info correctly from the "
         "input files and notify if incomplete information is present",
         "[reader]") {
  SECTION("Parcas complete information") {
    csaransh::Info lhs, rhs;
    lhs.substrate = "Fe";
    lhs.boxSize = 154.98;
    lhs.ncell = 54;
    lhs.energy = 10;
    lhs.xrec = -31.93;
    lhs.yrec = -31.65;
    lhs.zrec = -28.38;
    lhs.rectheta = 58.938;
    lhs.recphi = 47.339;
    lhs.origin = 0.25;
    lhs.structure = "bcc";
    lhs.latticeConst = 2.87;
    lhs.infile = "./data/parcas/005-md-10-1.in";
    lhs.name = "Fe_10_58-47";
    bool status = false;
    std::tie(rhs, status) = extractInfoParcas("./data/parcas/005-md-10-1.in");
    REQUIRE(status == true);
    CHECK(lhs.ncell == rhs.ncell);
    CHECK(lhs.boxSize == Approx(rhs.boxSize));
    CHECK(lhs.energy == Approx(rhs.energy));
    CHECK(lhs.rectheta == Approx(rhs.rectheta));
    CHECK(lhs.recphi == Approx(rhs.recphi));
    CHECK(lhs.xrec == Approx(rhs.xrec));
    CHECK(lhs.yrec == Approx(rhs.yrec));
    CHECK(lhs.zrec == Approx(rhs.zrec));
    CHECK(lhs.latticeConst == Approx(rhs.latticeConst));
    CHECK(lhs.origin == Approx(rhs.origin));
    CHECK(lhs.structure == rhs.structure);
    CHECK(lhs.infile == rhs.infile);
    CHECK(lhs.name == rhs.name);
    CHECK(lhs.substrate == rhs.substrate);
  }
  SECTION("Parcas incomplete information") {
    csaransh::Info rhs{};
    bool status = true;
    std::tie(rhs, status) =
        extractInfoParcas("./data/test/parcasIncomplete.err");
    REQUIRE(status == false);
  }
  SECTION("Lammps or disp complete information") {
    csaransh::Info lhs, rhs;
    lhs.substrate = "Fe";
    lhs.boxSize = 28.5532;
    lhs.ncell = 10;
    lhs.energy = 1;
    lhs.xrec = 14.2766;
    lhs.yrec = 14.2766;
    lhs.zrec = 14.2766;
    lhs.rectheta = 0.7854;
    lhs.recphi = 0.7854;
    lhs.origin = 0.0;
    lhs.structure = "bcc";
    lhs.latticeConst = 2.85532;
    lhs.infile = "./data/lammps/Pos1.in";
    lhs.name = "Fe_1_0-0";
    bool status = false;
    std::tie(rhs, status) = extractInfoLammps("./data/lammps/Pos1.in");
    REQUIRE(status == true);
    CHECK(lhs.ncell == rhs.ncell);
    CHECK(lhs.boxSize == Approx(rhs.boxSize));
    CHECK(lhs.energy == Approx(rhs.energy));
    CHECK(lhs.rectheta == Approx(rhs.rectheta));
    CHECK(lhs.recphi == Approx(rhs.recphi));
    CHECK(lhs.xrec == Approx(rhs.xrec));
    CHECK(lhs.yrec == Approx(rhs.yrec));
    CHECK(lhs.zrec == Approx(rhs.zrec));
    CHECK(lhs.latticeConst == Approx(rhs.latticeConst));
    CHECK(lhs.origin == Approx(rhs.origin));
    CHECK(lhs.structure == rhs.structure);
    CHECK(lhs.infile == rhs.infile);
    CHECK(lhs.name == rhs.name);
    CHECK(lhs.substrate == rhs.substrate);
  }
  SECTION("Lammps or disp incomplete information") {
    csaransh::Info rhs{};
    bool status = true;
    std::tie(rhs, status) =
        extractInfoLammps("./data/test/lammpsIncomplete.err");
    REQUIRE(status == false);
  }
  SECTION("Lammps invalid input") {
    csaransh::Info rhs{};
    bool status = true;
    std::tie(rhs, status) = extractInfoLammps("./data/test/lammpsInvalid.err");
    REQUIRE(status == false);
  }
  SECTION("Edge cases") {
    // file doesn't exist
    csaransh::Info rhs{};
    bool status = true;
    std::tie(rhs, status) = extractInfoLammps("file/not/found.err");
    REQUIRE(status == false);

    // wrong simulation code written in file
    std::tie(rhs, status) = extractInfoParcas("./data/lammps/Pos1.in");
    REQUIRE(status == false);
  }
}