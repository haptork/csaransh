#include <catch.hpp>

#include <infoReader.hpp>

using namespace csaransh;
using namespace std::string_literals;

SCENARIO("The user only needs to give xyz file for all the atoms or displaced "
         "atoms, the sofware should look for the input file with the similar "
         "name and fixed extension '.in' in the same directory, the special "
         "tag fpos in the xyz file name should be replaced by md in the input "
         "file once (and not in the path) (checks getInfileFromXyzfile, "
         "separateDirAndFile, separateFileAndExt",
         "[reader]") {
  SECTION("Normal cases - files that don't exist.") {
    REQUIRE(getInfileFromXyzfile("yellow.xyz") ==
            std::make_pair(""s, "yellow.xyz"s));
    REQUIRE(getInfileFromXyzfile("path/to/yellow.xyz") ==
            std::make_pair(""s, "yellow.xyz"s));
    REQUIRE(getInfileFromXyzfile("/path/to/yellow.disp") ==
            std::make_pair(""s, "yellow.disp"s));
    REQUIRE(getInfileFromXyzfile("./path/to/yellow.whatever") ==
            std::make_pair(""s, "yellow.whatever"s));
    REQUIRE(getInfileFromXyzfile("../path/to/yellow.xyz.whatever") ==
            std::make_pair(""s, "yellow.xyz.whatever"s));
  }
  SECTION("Normal cases - files that exist") {
    REQUIRE(getInfileFromXyzfile("./data/lammps/Pos1.xyz") ==
            std::make_pair("./data/lammps/Pos1.in"s, "Pos1.xyz"s));
    REQUIRE(getInfileFromXyzfile("./data/disp/Pos2.dispxyz") ==
            std::make_pair("./data/disp/common_input.in"s, "Pos2.dispxyz"s));
    REQUIRE(
        getInfileFromXyzfile("./data/parcas/005-fpos-10-1.xyz") ==
        std::make_pair("./data/parcas/005-md-10-1.in"s, "005-fpos-10-1.xyz"s));
  }

  SECTION("Edge cases") {
    REQUIRE(getInfileFromXyzfile("what") == std::make_pair(""s, "what"s));
    REQUIRE(getInfileFromXyzfile("what.") == std::make_pair(""s, "what."s));
  }
  SECTION("Edge cases - almost invalid inputs") { // might add warning to these
    REQUIRE(getInfileFromXyzfile("") == std::make_pair(""s, ""s));
    REQUIRE(getInfileFromXyzfile(".what") == std::make_pair(""s, ".what"s));
  }
}

SCENARIO("The simulation code type should be detected from the input file tag "
         "in first few lines (10 lines), the tags are: 'PARCAS', 'LAMMPS-XYZ', "
         "LAMMPS-DISP' all in capitals and as is. If no valid tags are present "
         "then it should be notified.",
         "[reader]") {
  SECTION("Positive cases: one of the simulation code tag is found") {
    REQUIRE(getSimulationCode("./data/parcas/005-md-10-1.in") ==
            std::make_pair(csaransh::XyzFileType::parcasWithStdHeader, true));
    REQUIRE(getSimulationCode("data/lammps/Pos1.in") ==
            std::make_pair(csaransh::XyzFileType::lammpsWithStdHeader, true));
    REQUIRE(
        getSimulationCode("./data/disp/common_input.in") ==
        std::make_pair(csaransh::XyzFileType::lammpsDisplacedCompute, true));
  }
  SECTION("Negative cases: no simulation code tag is found") {
    REQUIRE(getSimulationCode("./file/doesnt/exist") ==
            std::make_pair(csaransh::XyzFileType{}, false));
    REQUIRE(getSimulationCode("test/readerTest.cpp") ==
            std::make_pair(csaransh::XyzFileType{},
                           false)); // exists but not tag in top 10 lines
    REQUIRE(getSimulationCode("test/catchUnitTests.cpp") ==
            std::make_pair(csaransh::XyzFileType{},
                           false)); // exists but no tag at all
  }
}

SCENARIO("The software should read required simulation info correctly from the "
         "input files and notify if incomplete information is present",
         "[reader]") {
  SECTION("Parcas complete information") {
    csaransh::InputInfo lhs, rhs;
    csaransh::ExtraInfo lhsExtra, rhsExtra;
    lhsExtra.substrate = "Fe";
    lhs.boxSize = 154.98;
    lhs.ncell = 54;
    lhsExtra.energy = 10;
    lhsExtra.xrec = -31.93;
    lhsExtra.yrec = -31.65;
    lhsExtra.zrec = -28.38;
    lhsExtra.rectheta = 58.938;
    lhsExtra.recphi = 47.339;
    lhs.originX = 0.25;
    lhs.structure = "bcc";
    lhs.latticeConst = 2.87;
    lhs.temperature = 0;
    lhsExtra.infile= "./data/parcas/005-md-10-1.in";
    bool status = false;
    std::tie(rhs, rhsExtra, status) =
        extractInfoParcas("./data/parcas/005-md-10-1.in", "005-md-10-1.in");
    REQUIRE(status == true);
    CHECK(lhs.ncell == rhs.ncell);
    CHECK(lhs.boxSize == Approx(rhs.boxSize));
    CHECK(lhsExtra.energy == Approx(rhsExtra.energy));
    CHECK(lhsExtra.rectheta == Approx(rhsExtra.rectheta));
    CHECK(lhsExtra.recphi == Approx(rhsExtra.recphi));
    CHECK(lhsExtra.xrec == Approx(rhsExtra.xrec));
    CHECK(lhsExtra.yrec == Approx(rhsExtra.yrec));
    CHECK(lhsExtra.zrec == Approx(rhsExtra.zrec));
    CHECK(lhs.latticeConst == Approx(rhs.latticeConst));
    CHECK(lhs.originX == Approx(rhs.originX));
    CHECK(lhs.structure == rhs.structure);
    CHECK(lhsExtra.infile== rhsExtra.infile);
    CHECK(lhs.temperature == rhs.temperature);
    CHECK(lhsExtra.substrate == rhsExtra.substrate);
  }
  SECTION("Parcas incomplete information") {
    csaransh::InputInfo rhs{};
    csaransh::ExtraInfo rhsExtra{};
    bool status = true;
    std::tie(rhs, rhsExtra, status) = extractInfoParcas(
        "./data/test/parcasIncomplete.err", "parcasIncomplete.err");
    REQUIRE(status == false);
  }
  SECTION("Lammps or disp complete information") {
    csaransh::InputInfo lhs, rhs;
    csaransh::ExtraInfo lhsExtra, rhsExtra;
    lhsExtra.substrate = "Fe";
    lhs.ncell = 10;
    lhsExtra.energy = 1;
    lhs.originX = 0.0;
    lhs.structure = "bcc";
    lhs.latticeConst = 2.85532;
    lhsExtra.infile= "./data/lammps/Pos1.in";
    bool status = false;
    std::tie(rhs, rhsExtra, status) =
        extractInfoLammps("./data/lammps/Pos1.in", "Pos1.in");
    REQUIRE(status == true);
    CHECK(lhs.ncell == rhs.ncell);
    CHECK(lhsExtra.energy == Approx(rhsExtra.energy));
    CHECK(lhs.latticeConst == Approx(rhs.latticeConst));
    CHECK(lhs.originX == Approx(rhs.originX));
    CHECK(lhs.structure == rhs.structure);
    CHECK(lhsExtra.infile== rhsExtra.infile);
    CHECK(lhsExtra.substrate == rhsExtra.substrate);
  }
  SECTION("Lammps or disp incomplete information") {
    csaransh::InputInfo rhs{};
    csaransh::ExtraInfo rhsExtra{};
    bool status = true;
    std::tie(rhs, rhsExtra, status) =
        extractInfoLammps("./data/test/lammpsIncomplete.err", "lammsIncomp");
    REQUIRE(status == false);
  }
  SECTION("Lammps invalid input") {
    csaransh::InputInfo rhs{};
    csaransh::ExtraInfo rhsExtra{};
    bool status = true;
    std::tie(rhs, rhsExtra, status) =
        extractInfoLammps("./data/test/lammpsInvalid.err", "lammpsInvalid");
    REQUIRE(status == false);
  }
  SECTION("Edge cases") {
    // file doesn't exist
    csaransh::InputInfo rhs{};
    csaransh::ExtraInfo rhsExtra{};
    bool status = true;
    std::tie(rhs, rhsExtra, status) =
        extractInfoLammps("file/not/found.err", "found");
    REQUIRE(status == false);

    // wrong simulation code written in file
    std::tie(rhs, rhsExtra, status) =
        extractInfoParcas("./data/lammps/Pos1.in", "Pos");
    REQUIRE(status == false);
  }
}