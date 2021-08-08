#include <catch.hpp>

#include <AddOffset.hpp>
#include <NextExpected.hpp>
#include <results.hpp>
#include <xyz2defects.hpp>
#include <xyzReader.hpp>

#include <iostream>

using namespace csaransh;
SCENARIO("Find nearest lattice site for a coordinate given lattice structure - "
         "Addoffset",
         "[defectsTest]") {
  SECTION("Normal Cases - With 0.0 as origin") {
    auto origin = Coords{{0.0, 0.0, 0.0}};
    // Case 1
    AddOffset a{1.0, "bcc", origin};
    auto res = a(origin);
    REQUIRE(std::get<0>(res)[0] == Approx(0.0));
    REQUIRE(std::get<0>(res)[1] == Approx(0.0));
    REQUIRE(std::get<0>(res)[2] == Approx(0.0));
    REQUIRE(std::get<1>(res) == Approx(0.0));
    REQUIRE(std::get<2>(res) == origin);
    // Case a
    auto resa = a(Coords{{0.4, 0.5, 0.5}});
    REQUIRE(std::get<0>(resa)[0] == Approx(0.5));
    REQUIRE(std::get<0>(resa)[1] == Approx(0.5));
    REQUIRE(std::get<0>(resa)[2] == Approx(0.5));
    REQUIRE(std::get<1>(resa) == Approx(0.1));
    // Case b
    auto cb = Coords{{2.0, 1.5, 3.71}};
    auto resb = a(cb);
    CHECK(std::get<0>(resb)[0] == Approx(2.5));
    CHECK(std::get<0>(resb)[1] == Approx(1.5));
    CHECK(std::get<0>(resb)[2] == Approx(3.5));
    // checking if the lattice site found is nearer than other sites around
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 1.0, 3.0}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 2.0, 3.0}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 2.0, 4.0}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 1.0, 4.0}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{1.5, 1.5, 3.5}}));
    // Case c
    auto cc = Coords{{-0.1, -0.3, -0.71}};
    auto resc = a(cc);
    CHECK(std::get<0>(resc)[0] == Approx(0.0));
    CHECK(std::get<0>(resc)[1] == Approx(0.0));
    CHECK(std::get<0>(resc)[2] == Approx(-1.0));
    CHECK(std::get<1>(resc) <= calcDist(cc, Coords{{0.0, 0.0, 0.0}}));
    CHECK(std::get<1>(resc) <= calcDist(cc, Coords{{-0.5, -0.5, -0.5}}));
    CHECK(std::get<1>(resc) <= calcDist(cc, Coords{{0.0, 0.0, -1.0}}));
    CHECK(std::get<1>(resc) == Approx(0.429).epsilon(0.001));
    // Case d
    auto cd = Coords{{-0.3, -0.3, -0.71}};
    auto resd = a(cd);
    CHECK(std::get<0>(resd)[0] == Approx(-0.5));
    CHECK(std::get<0>(resd)[1] == Approx(-0.5));
    CHECK(std::get<0>(resd)[2] == Approx(-0.5));
    CHECK(std::get<1>(resd) <= calcDist(cd, Coords{{0.0, 0.0, 0.0}}));
    CHECK(std::get<1>(resd) <= calcDist(cd, Coords{{-1.0, -1.0, -1.0}}));
    CHECK(std::get<1>(resd) <= calcDist(cd, Coords{{0.0, 0.0, -1.0}}));
    CHECK(std::get<1>(resd) == Approx(0.352).epsilon(0.001));
  }
  SECTION("Normal Cases - With 0.25 as origin") {
    auto origin = Coords{{0.25, 0.25, 0.25}};
    // Case 1
    AddOffset a{1.0, "bcc", origin};
    auto res = a(origin);
    REQUIRE(std::get<0>(res)[0] == Approx(origin[0]));
    REQUIRE(std::get<0>(res)[1] == Approx(origin[1]));
    REQUIRE(std::get<0>(res)[2] == Approx(origin[2]));
    REQUIRE(std::get<1>(res) == Approx(0.0));
    REQUIRE(std::get<2>(res) == origin);
    // Case a
    auto resa = a(Coords{{0.64, 0.75, 0.75}});
    REQUIRE(std::get<0>(resa)[0] == Approx(0.75));
    REQUIRE(std::get<0>(resa)[1] == Approx(0.75));
    REQUIRE(std::get<0>(resa)[2] == Approx(0.75));
    REQUIRE(std::get<1>(resa) == Approx(0.11));
    // Case b
    auto cb = Coords{{2.0, 1.5, 3.71}};
    auto resb = a(cb);
    CHECK(std::get<0>(resb)[0] == Approx(1.75));
    CHECK(std::get<0>(resb)[1] == Approx(1.75));
    CHECK(std::get<0>(resb)[2] == Approx(3.75));
    // checking if the lattice site found is nearer than other sites around
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.25, 1.25, 3.25}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.25, 2.25, 3.25}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.25, 2.25, 4.25}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.25, 1.25, 4.25}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.75, 1.75, 3.75}}));
    // Case c
    auto cc = Coords{{-0.1, -0.3, -0.71}};
    auto resc = a(cc);
    CHECK(std::get<0>(resc)[0] == Approx(-0.25));
    CHECK(std::get<0>(resc)[1] == Approx(-0.25));
    CHECK(std::get<0>(resc)[2] == Approx(-0.25));
    CHECK(std::get<1>(resc) <= calcDist(cc, Coords{{0.25, 0.25, 0.25}}));
    CHECK(std::get<1>(resc) <= calcDist(cc, Coords{{-0.75, -0.75, -0.75}}));
    CHECK(std::get<1>(resc) <= calcDist(cc, Coords{{-0.25, -0.25, -1.25}}));
    CHECK(std::get<1>(resc) == Approx(0.486).epsilon(0.001));
    // Case d
    auto cd = Coords{{-0.3, -0.3, -0.71}};
    auto resd = a(cd);
    CHECK(std::get<0>(resd)[0] == Approx(-0.25));
    CHECK(std::get<0>(resd)[1] == Approx(-0.25));
    CHECK(std::get<0>(resd)[2] == Approx(-0.25));
    CHECK(std::get<1>(resd) <= calcDist(cd, Coords{{0.25, 0.25, 0.25}}));
    CHECK(std::get<1>(resd) <= calcDist(cd, Coords{{-0.75, -0.75, -0.75}}));
    CHECK(std::get<1>(resd) <= calcDist(cd, Coords{{-0.25, -0.25, -1.25}}));
    CHECK(std::get<1>(resd) == Approx(0.465).epsilon(0.001));
  }
  SECTION("Normal Cases - With 0.5 as origin and Fe lattice constant (2.85)") {
    auto origin = Coords{{0.5, 0.5, 0.5}};
    auto originMult = Coords{{0.5 * 2.85, 0.5 * 2.85, 0.5 * 2.85}};
    // Case 1
    AddOffset a{2.85, "bcc", origin};
    auto res = a(originMult);
    CHECK(std::get<0>(res)[0] == Approx(origin[0]));
    CHECK(std::get<0>(res)[1] == Approx(origin[1]));
    CHECK(std::get<0>(res)[2] == Approx(origin[2]));
    CHECK(std::get<1>(res) == Approx(0.0));
    CHECK(std::get<2>(res) == originMult);
    // Case a
    auto resa = a(Coords{{2.83, 2.85, 2.85}});
    REQUIRE(std::get<0>(resa)[0] == Approx(1.0));
    REQUIRE(std::get<0>(resa)[1] == Approx(1.0));
    REQUIRE(std::get<0>(resa)[2] == Approx(1.0));
    REQUIRE(std::get<1>(resa) == Approx(0.02));
    // Case b
    auto cb = Coords{{5.79, 3.98, 7.71}};
    auto resb = a(cb);
    CHECK(std::get<0>(resb)[0] == Approx(2.00));
    CHECK(std::get<0>(resb)[1] == Approx(1.00));
    CHECK(std::get<0>(resb)[2] == Approx(3.00));
    // checking if the lattice site found is nearer than other sites around
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{1.5, 1.5, 2.5}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 2.00, 3.00}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 2.0, 3.0}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.0, 1.0, 4.0}}));
    CHECK(std::get<1>(resb) <= calcDist(cb, Coords{{2.5, 1.5, 3.5}}));
  }
  SECTION("Edge Cases") {
    auto origin = Coords{{0.0, 0.0, 0.0}};
    // Case 1
    AddOffset a{1.0, "bcc", origin};
    auto c1 = Coords{{-0.0, -0.00001, 0.00001}};
    auto res = a(c1);
    REQUIRE(std::get<0>(res)[0] == Approx(0.0));
    REQUIRE(std::get<0>(res)[1] == Approx(0.0));
    REQUIRE(std::get<0>(res)[2] == Approx(0.0));
    REQUIRE(std::get<2>(res) == c1);
  }
}

SCENARIO("Enumerate all lattice sites for a bcc in ascending order given min "
         "and max lattice site. The lattice site are relative sites (i.e. "
         "latticeConst does not affect the values.) The min and max given "
         "should also be relative. The simulation codes like lammps / parcas "
         "build bcc lattice such that both the intertwined simple cubic "
         "lattices in bcc have same number of unit-cells. The code can assume "
         "that min and max are given accordingly in valid forms. The code "
         "should accomodate for change in origin according to the min value. - "
         "NextExptected",
         "[defectsTest]") {
  SECTION("Normal Case - 1") {
    // case 1
    auto origin = csaransh::Coords{{0., 0., 0.}};
    auto max = csaransh::Coords{{2.5, 2.5, 2.5}};
    auto maxInitial = getInitialMax(origin, max);
    NextExpected ne{
        origin, max,
        maxInitial}; // The min and max for two unit-cells with origin 0.0
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.cur() == Coords{{0.0, 0.0, 0.0}});
    CHECK(ne.increment() == Coords{{0.0, 0.0, 1.0}});
    CHECK(ne.increment() == Coords{{0.0, 0.0, 2.0}});
    CHECK(ne.increment() == Coords{{0.0, 1.0, 0.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{0.0, 1.0, 2.0}});
    CHECK(ne.increment() == Coords{{0.0, 2.0, 0.0}});
    CHECK(ne.increment() == Coords{{0.0, 2.0, 1.0}});
    CHECK(ne.increment() == Coords{{0.0, 2.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{0.5, 0.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 0.5, 2.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 1.5, 0.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 1.5, 1.5}});
    CHECK(ne.increment() == Coords{{0.5, 1.5, 2.5}});
    CHECK(ne.increment() == Coords{{0.5, 2.5, 0.5}});
    CHECK(ne.increment() == Coords{{0.5, 2.5, 1.5}});
    CHECK(ne.increment() == Coords{{0.5, 2.5, 2.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.0, 0.0, 0.0}});
    CHECK(ne.increment() == Coords{{1.0, 0.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 0.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.0, 1.0, 0.0}});
    CHECK(ne.increment() == Coords{{1.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 1.0, 2.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 0.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.5, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 0.5, 1.5}});
    CHECK(ne.increment() == Coords{{1.5, 0.5, 2.5}});
    CHECK(ne.increment() == Coords{{1.5, 1.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 1.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.5, 1.5, 2.5}});
    CHECK(ne.increment() == Coords{{1.5, 2.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 2.5, 1.5}});
    CHECK(ne.increment() == Coords{{1.5, 2.5, 2.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 0.0, 0.0}});
    CHECK(ne.increment() == Coords{{2.0, 0.0, 1.0}});
    CHECK(ne.increment() == Coords{{2.0, 0.0, 2.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 0.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 2.0, 0.0}});
    CHECK(ne.increment() == Coords{{2.0, 2.0, 1.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 2.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.5, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{2.5, 0.5, 1.5}});
    CHECK(ne.increment() == Coords{{2.5, 0.5, 2.5}});
    CHECK(ne.increment() == Coords{{2.5, 1.5, 0.5}});
    CHECK(ne.increment() == Coords{{2.5, 1.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.5, 1.5, 2.5}});
    CHECK(ne.increment() == Coords{{2.5, 2.5, 0.5}});
    CHECK(ne.increment() == Coords{{2.5, 2.5, 1.5}});
    CHECK(ne.increment() == Coords{{2.5, 2.5, 2.5}});
    REQUIRE(ne.allMax());
  }
  SECTION("Normal Case - 2") {
    // case 2
    auto origin = csaransh::Coords{{0.25, 0.25, 0.25}};
    auto max = csaransh::Coords{{2.75, 2.75, 2.75}};
    auto maxInitial = getInitialMax(origin, max);
    NextExpected ne{
        origin, max,
        maxInitial}; // The min and max for two unit-cells with origin 0.25
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.cur() == Coords{{0.25, 0.25, 0.25}});
    CHECK(ne.increment() == Coords{{0.25, 0.25, 1.25}});
    CHECK(ne.increment() == Coords{{0.25, 0.25, 2.25}});
    CHECK(ne.increment() == Coords{{0.25, 1.25, 0.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.25, 1.25, 1.25}});
    CHECK(ne.increment() == Coords{{0.25, 1.25, 2.25}});
    CHECK(ne.increment() == Coords{{0.25, 2.25, 0.25}});
    CHECK(ne.increment() == Coords{{0.25, 2.25, 1.25}});
    CHECK(ne.increment() == Coords{{0.25, 2.25, 2.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.75, 0.75, 0.75}});
    CHECK(ne.increment() == Coords{{0.75, 0.75, 1.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.75, 0.75, 2.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.75, 1.75, 0.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.75, 1.75, 1.75}});
    CHECK(ne.increment() == Coords{{0.75, 1.75, 2.75}});
    CHECK(ne.increment() == Coords{{0.75, 2.75, 0.75}});
    CHECK(ne.increment() == Coords{{0.75, 2.75, 1.75}});
    CHECK(ne.increment() == Coords{{0.75, 2.75, 2.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.25, 0.25, 0.25}});
    CHECK(ne.increment() == Coords{{1.25, 0.25, 1.25}});
    CHECK(ne.increment() == Coords{{1.25, 0.25, 2.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.25, 1.25, 0.25}});
    CHECK(ne.increment() == Coords{{1.25, 1.25, 1.25}});
    CHECK(ne.increment() == Coords{{1.25, 1.25, 2.25}});
    CHECK(ne.increment() == Coords{{1.25, 2.25, 0.25}});
    CHECK(ne.increment() == Coords{{1.25, 2.25, 1.25}});
    CHECK(ne.increment() == Coords{{1.25, 2.25, 2.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.75, 0.75, 0.75}});
    CHECK(ne.increment() == Coords{{1.75, 0.75, 1.75}});
    CHECK(ne.increment() == Coords{{1.75, 0.75, 2.75}});
    CHECK(ne.increment() == Coords{{1.75, 1.75, 0.75}});
    CHECK(ne.increment() == Coords{{1.75, 1.75, 1.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.75, 1.75, 2.75}});
    CHECK(ne.increment() == Coords{{1.75, 2.75, 0.75}});
    CHECK(ne.increment() == Coords{{1.75, 2.75, 1.75}});
    CHECK(ne.increment() == Coords{{1.75, 2.75, 2.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.25, 0.25, 0.25}});
    CHECK(ne.increment() == Coords{{2.25, 0.25, 1.25}});
    CHECK(ne.increment() == Coords{{2.25, 0.25, 2.25}});
    CHECK(ne.increment() == Coords{{2.25, 1.25, 0.25}});
    CHECK(ne.increment() == Coords{{2.25, 1.25, 1.25}});
    CHECK(ne.increment() == Coords{{2.25, 1.25, 2.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.25, 2.25, 0.25}});
    CHECK(ne.increment() == Coords{{2.25, 2.25, 1.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.25, 2.25, 2.25}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.75, 0.75, 0.75}});
    CHECK(ne.increment() == Coords{{2.75, 0.75, 1.75}});
    CHECK(ne.increment() == Coords{{2.75, 0.75, 2.75}});
    CHECK(ne.increment() == Coords{{2.75, 1.75, 0.75}});
    CHECK(ne.increment() == Coords{{2.75, 1.75, 1.75}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.75, 1.75, 2.75}});
    CHECK(ne.increment() == Coords{{2.75, 2.75, 0.75}});
    CHECK(ne.increment() == Coords{{2.75, 2.75, 1.75}});
    CHECK(ne.increment() == Coords{{2.75, 2.75, 2.75}});
    REQUIRE(ne.allMax());
  }
  SECTION("Edge Cases - 1") {
    // case 2
    auto origin = csaransh::Coords{{0.5, 0.5, 0.5}};
    auto max = csaransh::Coords{{2.00, 2.00, 2.00}};
    auto maxInitial = getInitialMax(origin, max);
    NextExpected ne{origin, max, maxInitial};
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 0.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 1.5, 0.5}});
    CHECK(ne.increment() == Coords{{0.5, 1.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 1.0, 2.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.5, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 0.5, 1.5}});
    CHECK(ne.increment() == Coords{{1.5, 1.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 1.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 2.0}});
    CHECK(ne.increment() == Coords{{2.0, 2.0, 1.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 2.0, 2.0}});
    REQUIRE(ne.allMax());
  }
  SECTION("Edge Cases - Almost invalid input - 1") {
    // case 1
    auto origin = csaransh::Coords{{0.0, 0.0, 0.0}};
    auto max = csaransh::Coords{
        {2.00, 2.00, 2.00}}; // a valid max should have been 2.5, orign 0.0
    auto maxInitial = getInitialMax(origin, max);
    NextExpected ne{origin, max, maxInitial};
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.cur() == Coords{{0.0, 0.0, 0.0}});
    CHECK(ne.increment() == Coords{{0.0, 0.0, 1.0}});
    CHECK(ne.increment() == Coords{{0.0, 0.0, 2.0}});
    CHECK(ne.increment() == Coords{{0.0, 1.0, 0.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{0.0, 1.0, 2.0}});
    CHECK(ne.increment() == Coords{{0.0, 2.0, 0.0}});
    CHECK(ne.increment() == Coords{{0.0, 2.0, 1.0}});
    CHECK(ne.increment() == Coords{{0.0, 2.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{0.5, 0.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 1.5, 0.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{0.5, 1.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.0, 0.0, 0.0}});
    CHECK(ne.increment() == Coords{{1.0, 0.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 0.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.0, 1.0, 0.0}});
    CHECK(ne.increment() == Coords{{1.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 1.0, 2.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 0.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 1.0}});
    CHECK(ne.increment() == Coords{{1.0, 2.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{1.5, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 0.5, 1.5}});
    CHECK(ne.increment() == Coords{{1.5, 1.5, 0.5}});
    CHECK(ne.increment() == Coords{{1.5, 1.5, 1.5}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 0.5, 0.5}});
    CHECK(ne.increment() == Coords{{2.0, 0.5, 1.5}});
    /*
    CHECK(ne.increment() == Coords{{2.0, 0.0, 1.0}});
    CHECK(ne.increment() == Coords{{2.0, 0.0, 2.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 0.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 1.0}});
    CHECK(ne.increment() == Coords{{2.0, 1.0, 2.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 2.0, 0.0}});
    CHECK(ne.increment() == Coords{{2.0, 2.0, 1.0}});
    REQUIRE_FALSE(ne.allMax());
    CHECK(ne.increment() == Coords{{2.0, 2.0, 2.0}});
    //REQUIRE(ne.max() ==  Coords{{0.0,0.0,0.0}});
    REQUIRE(ne.allMax());
    */
  }
}

TEST_CASE("Read atom coordinates from a parcas xyz file line",
          "[defectsTest]") {
  SECTION("Normal cases") {
    Coords c;
    csaransh::lineStatus ls;
    // coords
    std::tie(ls, c) = csaransh::getCoordParcas("Fe   -76.770403   +7.2e2   .7",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{-76.770403, 720, 0.7}});
    std::tie(ls, c) =
        csaransh::getCoordParcas("what   +76.770403   -7.2e2   0.700 Frame",
                                 csaransh::frameStatus::inFrame, 2);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{76.770403, -720, 0.7}});
    std::tie(ls, c) = csaransh::getCoordParcas(
        "  what 0.000000 +7.2e2 3f whatever  ", csaransh::frameStatus::inFrame, 2);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{0.0, 720, 3.0}});
    // garbage
    std::tie(ls, c) =
        csaransh::getCoordParcas("53   +76.770403   -7.2e2   0.700 Frame",
                                 csaransh::frameStatus::inFrame, 2);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) =
        csaransh::getCoordParcas("garbage", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordParcas("-76.770403   +7.2e2   .7",
                                               csaransh::frameStatus::inFrame, 1);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{-76.770403, +7.2e2, .7}});
    std::tie(ls, c) =
        csaransh::getCoordParcas("what 34 2.5", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordParcas(
        "-76.770403   +7.2e2   .7 garbage", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordParcas("what 34 2.5 garbage",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordParcas("what Frame 2.5 garbage",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    // border
    std::tie(ls, c) = csaransh::getCoordParcas("Frame 2.5 garbage",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::frameBorder);
    std::tie(ls, c) =
        csaransh::getCoordParcas("Frame", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::frameBorder);
    std::tie(ls, c) = csaransh::getCoordParcas("  Frame 0.000000 +7.2e2 3f",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::frameBorder);
  }
}

TEST_CASE("Read atom coordinates from a line from lammps xyz file",
          "[defectsTest]") {
  SECTION("Normal cases") {
    Coords c;
    csaransh::lineStatus ls;
    // coords
    std::tie(ls, c) = csaransh::getCoordLammps("Fe   -76.770403   +7.2e2   .7",
                                               csaransh::frameStatus::inFrame, 2);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{-76.770403, 720, 0.7}});
    std::tie(ls, c) =
        csaransh::getCoordLammps("54   +76.770403   -7.2e2   0.700 ITEM:",
                                 csaransh::frameStatus::inFrame, 2);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{76.770403, -720, 0.7}});
    std::tie(ls, c) = csaransh::getCoordLammps(
        "  what if 0.000000 +7.2e2 3f whatever  ", csaransh::frameStatus::inFrame, 3);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{0.0, 720, 3.0}});
    std::tie(ls, c) = csaransh::getCoordLammps(
        "no  what if 0.000000 +7.2e2 3f 2.0 ", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == Coords{{720, 3.0, 2.0}});
    // garbage
    std::tie(ls, c) = csaransh::getCoordLammps(
        "no  what if 0.000000 +7.2e2 3f 2.0 whatever  ", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) =
        csaransh::getCoordLammps("garbage", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordLammps("-76.770403   +7.2e2   .7",
                                               csaransh::frameStatus::inFrame, 2);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) =
        csaransh::getCoordLammps("what 34 2.5", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordLammps(
        "-76.770403   +7.2e2   .7 garbage", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordLammps("what 34 2.5 garbage",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordLammps("what ITEM: 2.5 garbage",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::garbage);
    // border
    std::tie(ls, c) = csaransh::getCoordLammps("ITEM: 2.5 garbage",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::frameBorder);
    std::tie(ls, c) =
        csaransh::getCoordLammps("ITEM:", csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::frameBorder);
    std::tie(ls, c) = csaransh::getCoordLammps("  ITEM: 0.000000 +7.2e2 3f",
                                               csaransh::frameStatus::inFrame, 0);
    CHECK(ls == csaransh::lineStatus::frameBorder);
  }
}

TEST_CASE(
    "Read displaced (interstitial and vacancy pair) coordinates from a line",
    "[defectsTest]") {
  SECTION("Normal cases") {
    std::array<Coords, 2> c;
    csaransh::lineStatus ls;
    // coords
    std::tie(ls, c) =
        csaransh::getCoordDisplaced("1 -76.770403   +7.2e2   .7  +76.770403   "
                                    "-7.2e2   0.700 6891 112087 1 ");
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == std::array<Coords, 2>{{Coords{{-76.770403, 720, 0.7}},
                                      Coords{{76.770403, -720, 0.7}}}});
    std::tie(ls, c) = csaransh::getCoordDisplaced(
        "Fe +76.770403   -7.2e2   0.700 -76.770403  +7.2e2   .7  ");
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == std::array<Coords, 2>{{Coords{{76.770403, -720, 0.7}},
                                      Coords{{-76.770403, 720, 0.7}}}});
    std::tie(ls, c) = csaransh::getCoordDisplaced(
        "  Fe +76.770403f   -7.2e2d   0.700 -76.770403  +7.2e2   .7  ITEM:");
    CHECK(ls == csaransh::lineStatus::coords);
    CHECK(c == std::array<Coords, 2>{{Coords{{76.770403, -720, 0.7}},
                                      Coords{{-76.770403, 720, 0.7}}}});
    // garbage
    std::tie(ls, c) = csaransh::getCoordDisplaced("garbage");
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordDisplaced(
        "Fe   -76.770403   +7.2e2 -76.770403   +7.2e2   .7");
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordDisplaced(
        "Fe   -76.770403   +7.2e2-76.770403   +7.2e2   .7");
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) =
        csaransh::getCoordDisplaced("what 34 2.5 3.5 whatever 3.0 3.5");
    CHECK(ls == csaransh::lineStatus::garbage);
    std::tie(ls, c) = csaransh::getCoordDisplaced("what ITEM: 2.5 garbage");
    CHECK(ls == csaransh::lineStatus::garbage);
    // border
    std::tie(ls, c) = csaransh::getCoordDisplaced("ITEM: ENTRIES 2.5 garbage");
    CHECK(ls == csaransh::lineStatus::frameBorder);
    std::tie(ls, c) = csaransh::getCoordDisplaced("ITEM: ENTRIES");
    CHECK(ls == csaransh::lineStatus::frameBorder);
    std::tie(ls, c) = csaransh::getCoordDisplaced(
        " ITEM: +76.770403   -7.2e2   0.700 -76.770403  +7.2e2   .7");
    CHECK(ls == csaransh::lineStatus::garbage);
  }
}

SCENARIO("Given xyz coordinates of all the lattice atoms, output only the "
         "defects, labelled by interstitial and vacancy, annihilated (psuedo) "
         "or actual (true)",
         "[defectsTest]") {
  SECTION("Normal cases") {
    SECTION("Single Dumbbell") {
      std::vector<std::tuple<Coords, double, Coords>> atoms;
      auto origin = csaransh::Coords{{0.25, 0.25, 0.25}};
      auto max = csaransh::Coords{{10.75, 10.75, 10.75}};
      auto maxInitial = getInitialMax(origin, max);
      NextExpected ne{
          origin, max,
          maxInitial}; // The min and max for two unit-cells with origin 0.0
      auto latticeConst = 2.85;
      AddOffset addOffset{latticeConst, "bcc", origin}; // Fe
      Coords interstitialCoord, vacancyCoord;
      auto i = 0;
      auto pickAt = 30; // this is ~ 10th atom
      while (true) {
        Coords c = ne.cur();
        // if (i != pickAt) {
        for (auto &jt : c) {
          jt *= latticeConst;
          jt += (i++ & 1) ? -0.15 : 0.15;
        }
        //}
        if (i == pickAt) {
          vacancyCoord = ne.cur();
          for (auto &jt : vacancyCoord)
            jt *= latticeConst;
          auto picked = atoms[pickAt / 5];
          for (int k = 0; k < 3; ++k) {
            auto diff = std::get<0>(picked)[k] - std::get<2>(picked)[k];
            c[k] = std::get<0>(picked)[k] - diff + 0.001;
          }
          interstitialCoord = c;
        }
        atoms.emplace_back(addOffset(c));
        if (ne.allMax()) break;
        ne.increment();
      }
      InputInfo info;
      Config config;
      config.safeRunChecks = false;
      config.isIgnoreBoundaryDefects = false;
      info.latticeConst = latticeConst;
      info.originX = origin[0];
      info.originY = origin[1];
      info.originZ = origin[2];
      ExtraInfo extraInfo;
      auto fsAtoms = std::make_pair(csaransh::xyzFileStatus::reading, atoms);
      auto ungroupedDefectsDumbbellPair = atoms2defects(fsAtoms, info, extraInfo, config);
      auto ungroupedDefects = std::get<2>(ungroupedDefectsDumbbellPair);
      REQUIRE(ungroupedDefects.size() == 4); // 2 interstitials, 2 vacancies
      SECTION("Check cluster grouping") {
        int nDefects;
        double inClusterFractionI, inClusterFractionV;
        auto defects = groupDefects(ungroupedDefects, latticeConst);
        auto clusterSizeMap = clusterSizes(defects);
        REQUIRE(clusterSizeMap.size() ==
                2); // one cluster of three and other of one
        SECTION("Check ndefects and cluster sizes") {
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 1);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(100.0));
          ignoreSmallClusters(defects, clusterSizeMap);
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 1);
          REQUIRE(inClusterFractionI == Approx(0.0));
          REQUIRE(inClusterFractionV == Approx(0.0)); // changed
          auto clusterIdMap = csaransh::clusterMapping(defects);
          REQUIRE(clusterIdMap.size() == 0); // 1 dumbbell
          auto clusterIVMap =
              csaransh::clusterIVType(clusterIdMap, clusterSizeMap);
          REQUIRE(clusterIVMap.size() == 0);
          int maxClusterSizeI, maxClusterSizeV;
          std::tie(maxClusterSizeI, maxClusterSizeV) =
              csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
          REQUIRE(maxClusterSizeI == 0);
          REQUIRE(maxClusterSizeV == 0);
          /*
        for (auto x : defects) {
          for (auto c : std::get<0>(x)) std::cout << c << ", ";
          std::cout << std::get<1>(x) << ", " << std::get<2>(x) << ", " << std::get<3>(x);
          std::cout << std::endl;
        }
        */

          SECTION("Check cluster features") {
            auto feats = csaransh::clusterFeatures(
                defects, clusterIdMap, clusterSizeMap, latticeConst);
            REQUIRE(feats.size() == 0);
            /*
            const auto &distFeat = std::get<0>(std::begin(feats)->second);
            REQUIRE(distFeat[0] + distFeat[distFeat.size() - 1] == 1.0); // TODO
            REQUIRE(distFeat[0] == Approx(2.0 / 6.0)); // TODO
            // REQUIRE(distFeat[distFeat.size() - 1] == Approx(4.0 / 6.0));
            REQUIRE(std::accumulate(begin(distFeat), end(distFeat), 0.0) ==
                    Approx(1.0));
            const auto &angleFeat = std::get<1>(std::begin(feats)->second);
            // REQUIRE(angleFeat[0] + angleFeat[angleFeat.size() - 1] == Approx(1.0));
            REQUIRE(std::accumulate(begin(angleFeat), end(angleFeat), 0.0) ==
                    Approx(1.0));
            const auto &adjFeat = std::get<2>(std::begin(feats)->second);
            REQUIRE(adjFeat[2] == Approx(1.0));
            REQUIRE(std::accumulate(begin(adjFeat), end(adjFeat), 0.0) ==
                    Approx(1.0));
                    */
          }
        }   // ndefects and cluster sizes
      }     // cluster grouping
    }       // End of Single Dumbbell test
    SECTION("big interstitial cluster") {
      std::vector<std::tuple<Coords, double, Coords>> atoms;
      auto origin = csaransh::Coords{{0.5, 0.5, 0.5}};
      auto max = csaransh::Coords{{6.0, 6.0, 6.0}};
      auto maxInitial = getInitialMax(origin, max);
      NextExpected ne{
          origin, max,
          maxInitial}; // The min and max for two unit-cells with origin 0.0
      auto latticeConst = 3.165; // W
      AddOffset addOffset{latticeConst, "bcc", origin};
      Coords lastInterstitialCoord, lastVacancyCoord;
      auto i = 0;
      auto pickAt = 300; // this is ~ 100th atom
      while (true) {
        // std::cout << ne.cur()[0] << ", " << ne.cur()[1] << ", " <<
        // ne.cur()[2] << " | " << ne.minCur()[0] << ", " << ne.maxCur()[0] <<
        // '\n';
        Coords c = ne.cur();
        // if (i != pickAt) {
        for (auto &jt : c) {
          jt *= latticeConst;
          jt += (i++ & 1) ? -0.16 : 0.16;
        }
        //}
        if (i % pickAt == 0) {
          lastVacancyCoord = ne.cur();
          for (auto &jt : lastVacancyCoord)
            jt *= latticeConst;
          auto picked = atoms[pickAt / 5];
          for (int k = 0; k < 3; ++k) {
            auto diff = std::get<0>(picked)[k] - std::get<2>(picked)[k];
            c[k] = std::get<0>(picked)[k] - diff + (0.001 * (i / pickAt));
          }
          lastInterstitialCoord = c;
        }
        atoms.emplace_back(addOffset(c));
        if (ne.allMax()) break;
        ne.increment();
      }
      InputInfo info;
      Config config;
      config.safeRunChecks = false;
      config.isIgnoreBoundaryDefects = false;
      info.latticeConst = latticeConst;
      info.originX = origin[0];
      info.originY = origin[1];
      info.originZ = origin[2];
      ExtraInfo extraInfo;
      auto fsAtoms = std::make_pair(csaransh::xyzFileStatus::reading, atoms);
      auto ungroupedDefectsDumbbellPair = atoms2defects(fsAtoms, info, extraInfo, config);
      auto ungroupedDefects = std::get<2>(ungroupedDefectsDumbbellPair);
      REQUIRE(ungroupedDefects.size() == 10);
      int nDefects;
      double inClusterFractionI, inClusterFractionV;
      std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
          csaransh::getNDefectsAndClusterFractions(ungroupedDefects);
      SECTION("Check cluster grouping") {
        auto defects = groupDefects(ungroupedDefects, latticeConst);
        auto clusterSizeMap = clusterSizes(defects);
        REQUIRE(clusterSizeMap.size() == 5);
        SECTION("Check ndefects and cluster sizes") {
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 4);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(100.0));
          ignoreSmallClusters(defects, clusterSizeMap);
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 4);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(0.0)); // changed
          auto clusterIdMap = csaransh::clusterMapping(defects);
          REQUIRE(clusterIdMap.size() == 1); // 1 interstitial cluster
          REQUIRE(std::begin(clusterIdMap)->second.size() == 6);
          auto clusterIVMap =
              csaransh::clusterIVType(clusterIdMap, clusterSizeMap);
          REQUIRE(clusterIVMap.size() == 1);
          REQUIRE(std::begin(clusterIVMap)->second == 4); // surviving
          int maxClusterSizeI, maxClusterSizeV;
          std::tie(maxClusterSizeI, maxClusterSizeV) =
              csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
          REQUIRE(maxClusterSizeI == 4);
          REQUIRE(maxClusterSizeV == 0);
        } // ndefects and cluster sizes
      }   // cluster grouping
    }
    SECTION("Interstitial and Vacancy in two big clusters") {
      std::vector<std::tuple<Coords, double, Coords>> atoms;
      auto origin = csaransh::Coords{{0.0, 0.0, 0.0}};
      auto max = csaransh::Coords{{4.5, 4.5, 4.5}};
      auto maxInitial = getInitialMax(origin, max);
      NextExpected ne{
          origin, max,
          maxInitial}; // The min and max for two unit-cells with origin 0.0
      auto latticeConst = 3.165; // W
      AddOffset addOffset{latticeConst, "bcc", Coords{0., 0., 0.}};
      Coords lastInterstitialCoord, lastVacancyCoord;
      auto i = 0;
      auto pickAt = 300; // this is ~ 100th atom
      while (true) {
        Coords c = ne.cur();
        // if (i != pickAt) {
        for (auto &jt : c) {
          jt *= latticeConst;
          jt += (i++ & 1) ? -0.16 : 0.16;
        }
        //}
        if (i > pickAt && i / pickAt < 2) {
          lastVacancyCoord = ne.cur();
          for (auto &jt : lastVacancyCoord)
            jt *= latticeConst;
          auto picked = atoms[pickAt / 50];
          for (int k = 0; k < 3; ++k) {
            auto diff = std::get<0>(picked)[k] - std::get<2>(picked)[k];
            c[k] = std::get<0>(picked)[k] - diff + (0.0001 * (i - pickAt));
          }
          lastInterstitialCoord = c;
        }
        atoms.emplace_back(addOffset(c));
        if (ne.allMax()) break;
        ne.increment();
      }
      InputInfo info;
      Config config;
      config.isIgnoreBoundaryDefects = false;
      config.safeRunChecks = false;
      config.isIgnoreBoundaryDefects = false;
      info.latticeConst = latticeConst;
      info.originX = origin[0];
      info.originY = origin[1];
      info.originZ = origin[2];
      ExtraInfo extraInfo;
      auto fsAtoms = std::make_pair(csaransh::xyzFileStatus::reading, atoms);
      auto ungroupedDefectsDumbbellPair = atoms2defects(fsAtoms, info, extraInfo, config);
      auto ungroupedDefects = std::get<2>(ungroupedDefectsDumbbellPair);
      CHECK(ungroupedDefects.size() == 200);
      int nDefects;
      double inClusterFractionI, inClusterFractionV;
      std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
          csaransh::getNDefectsAndClusterFractions(ungroupedDefects);
      SECTION("Check cluster grouping") {
        auto defects = groupDefects(ungroupedDefects, latticeConst);
        /*
        for (auto x : defects) {
          for (auto c : std::get<0>(x)) std::cout << c << ", ";
          std::cout << std::get<1>(x) << ", " << std::get<2>(x) << ", " << std::get<3>(x);
          std::cout << std::endl;
        }
        */
        auto clusterSizeMap = clusterSizes(defects);
        REQUIRE(clusterSizeMap.size() == 2);  // TODO:  unimportant! check again
        SECTION("Check ndefects and cluster sizes") {
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 99);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(100.0));
          ignoreSmallClusters(defects, clusterSizeMap);
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 99);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(100.0));
          auto clusterIdMap = csaransh::clusterMapping(defects);
          REQUIRE(clusterIdMap.size() == 2); // 1 interstitial and 1 vacancy cluster
          auto it = std::begin(clusterIdMap);
          // REQUIRE((it->second.size() == 98 || it->second.size() == 104));
          CHECK(it->second.size() == 99);
          it++;
          CHECK(it->second.size() == 101);
          auto clusterIVMap =
              csaransh::clusterIVType(clusterIdMap, clusterSizeMap);
          REQUIRE(clusterIVMap.size() == 2);
          auto jt = std::begin(clusterIVMap);
          REQUIRE(std::abs(jt->second) == 99); // surviving
          jt++;
          REQUIRE(std::abs(jt->second) == 99); // surviving
          int maxClusterSizeI, maxClusterSizeV;
          std::tie(maxClusterSizeI, maxClusterSizeV) =
              csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
          REQUIRE(maxClusterSizeI == 99);
          REQUIRE(maxClusterSizeV == 99);
        } // ndefects and cluster sizes
      }   // cluster grouping
    }
  }
  SECTION("Invalid input cases") {
    SECTION(
        "One atom kicked out of the box : unwrapped coordinates are invalid") {
      std::vector<std::tuple<Coords, double, Coords>> atoms;

      auto origin = csaransh::Coords{{0.25, 0.25, 0.25}};
      auto max = csaransh::Coords{{10.75, 10.75, 10.75}};
      auto maxInitial = getInitialMax(origin, max);
      NextExpected ne{
          origin, max,
          maxInitial}; // The min and max for two unit-cells with origin 0.0
      REQUIRE_FALSE(ne.allMax());
      auto latticeConst = 2.85;
      AddOffset addOffset{latticeConst, "bcc", origin}; // Fe
      auto i = 0;
      auto pickAt = 30; // this is ~ 10th atom
      while (true) {
        Coords c = ne.cur();
        // if (i != pickAt) {
        for (auto &jt : c) {
          jt *= latticeConst;
          jt += (i++ & 1) ? -0.15 : 0.15;
        }
        //}
        if (i == pickAt) {
          for (int k = 0; k < 3; ++k) {
            c[k] += latticeConst * 11;
          }
        }
        atoms.emplace_back(addOffset(c));
        if (ne.allMax()) break;
        ne.increment();
      }
      InputInfo info;
      Config config;
      config.safeRunChecks = false;
      info.latticeConst = latticeConst;
      info.originX = origin[0];
      info.originY = origin[1];
      info.originZ = origin[2];
      ExtraInfo extraInfo;
      auto fsAtoms = std::make_pair(csaransh::xyzFileStatus::reading, atoms);
      auto ungroupedDefectsDumbbellPair = atoms2defects(fsAtoms, info, extraInfo, config);
      auto ungroupedDefects = std::get<2>(ungroupedDefectsDumbbellPair);
      // it should have been 4 but now alot more defects are counted as the
      // box size is inferred from the atom coordinates, including the atom
      // that we kicked out of the box
      REQUIRE(ungroupedDefects.size() > 4);
      REQUIRE(ungroupedDefects.size() >
              400); // even greater than 100 times of actual
      auto interstitialCount = 0;
      auto vacancyCount = 0;
      for (const auto &it : ungroupedDefects) {
        if (DefectTWrap::isInterstitial(it))
          interstitialCount++;
        else
          vacancyCount++;
      }
      REQUIRE(vacancyCount > interstitialCount);
      REQUIRE(interstitialCount == 0);
    }
  }
  SECTION("Edge cases") {
    SECTION("Perfect lattice") {
      std::vector<std::tuple<Coords, double, Coords>> atoms;
      auto origin = csaransh::Coords{{0.0, 0.0, 0.0}};
      auto max = csaransh::Coords{{1.5, 1.5, 1.5}};
      auto maxInitial = getInitialMax(origin, max);
      NextExpected ne{
          origin, max,
          maxInitial}; // The min and max for two unit-cells with origin 0.0
      AddOffset addOffset{1.0, "bcc", origin};
      while (true) {
        atoms.emplace_back(addOffset(ne.cur()));
        if (ne.allMax()) break;
        ne.increment();
      }
      InputInfo info;
      Config config;
      config.safeRunChecks = false;
      info.latticeConst = 1.0;
      info.originX = origin[0];
      info.originY = origin[1];
      info.originZ = origin[2];
      ExtraInfo extraInfo;
      auto fsAtoms = std::make_pair(csaransh::xyzFileStatus::reading, atoms);
      auto ungroupedDefectsDumbbellPair = atoms2defects(fsAtoms, info, extraInfo, config);
      REQUIRE(std::get<2>(ungroupedDefectsDumbbellPair).empty());
    }
    SECTION("slightly shaken lattice") {
      std::vector<std::tuple<Coords, double, Coords>> atoms;
      auto origin = csaransh::Coords{{0.0, 0.0, 0.0}};
      auto max = csaransh::Coords{{1.5, 1.5, 1.5}};
      auto maxInitial = getInitialMax(origin, max);
      NextExpected ne{
          origin, max,
          maxInitial}; // The min and max for two unit-cells with origin 0.0
      AddOffset addOffset{1.0, "bcc", Coords{0.0, 0.0, 0.0}};
      auto i = 0;
      while (true) {
        Coords c = ne.cur();
        for (auto &jt : c) {
          jt += (i++ & 1) ? -0.1 : 0.15;
        }
        atoms.emplace_back(addOffset(c));
        if (ne.allMax()) break;
        ne.increment();
      }
      InputInfo info;
      ExtraInfo extraInfo;
      Config config;
      config.safeRunChecks = false;
      info.latticeConst = 1.0;
      info.originX = origin[0];
      info.originY = origin[1];
      info.originZ = origin[2];
      auto fsAtoms = std::make_pair(csaransh::xyzFileStatus::reading, atoms);
      auto ungroupedDefectsDumbbellPair = atoms2defects(fsAtoms, info, extraInfo, config);
      REQUIRE(std::get<2>(ungroupedDefectsDumbbellPair).empty());
    }
  }
}
SCENARIO("Given xyz coordinates of all the displaced atoms, output only the "
         "defects, "
         "labelled by interstitial and vacancy, annihilated (psuedo) or actual "
         "(true)",
         "[defectsTest]") {
  SECTION("Normal cases") {
    SECTION("Ring") {
      std::vector<std::array<csaransh::Coords, 2>> displacedOld{
          {{Coords{{-155.886, 2.3739, -32.4433}},
            Coords{{-155.652, 1.48622, -33.0678}}}},
          {{Coords{{-154.304, 3.9565, -30.8607}},
            Coords{{-155.42, 3.22437, -31.5164}}}},
          {{Coords{{-152.721, -0.7913, -32.4433}},
            Coords{{-153.627, -0.374023, -33.1668}}}},
          {{Coords{{-152.721, 2.3739, -29.2781}},
            Coords{{-151.911, -0.262889, -31.6111}}}},
          {{Coords{{-152.721, 5.5391, -32.4433}},
            Coords{{-151.88, 1.59063, -29.7559}}}},
          {{Coords{{-151.138, 0.7913, -30.8607}},
            Coords{{-153.531, 3.27719, -29.6802}}}},
          {{Coords{{-149.556, 2.3739, -32.4433}},
            Coords{{-151.784, 5.40975, -33.1103}}}},
          {{Coords{{-154.304, 0.7913, -34.0259}},
            Coords{{-153.606, 5.09886, -31.6038}}}},
          {{Coords{{-152.721, 2.3739, -35.6085}},
            Coords{{-150.064, 1.60602, -31.6352}}}},
          {{Coords{{-151.138, 3.9565, -34.0259}},
            Coords{{-149.965, 3.26968, -33.2595}}}},
          {{Coords{{-168.547, 2.3739, -16.6173}},
            Coords{{-153.783, 1.57628, -34.8461}}}},
          {{Coords{{-166.964, -30.8607, -27.6955}},
            Coords{{-152.263, 1.98476, -36.8897}}}},
          {{Coords{{-163.799, -113.156, 19.7825}},
            Coords{{-151.98, 3.48003, -34.8472}}}}};
      std::array<std::vector<csaransh::Coords>, 2> displaced;
      for (const auto &x : displacedOld) {
        displaced[0].emplace_back(std::move(x[0]));
        displaced[1].emplace_back(std::move(x[1]));
      }
      auto fs = csaransh::xyzFileStatus::reading;
      auto fsDisplaced = std::make_pair(fs, displaced);
      auto latticeConst = 3.165;
      auto ungroupedDefectsDumbbellPair =
          displacedAtoms2defects(fsDisplaced, latticeConst);
      auto ungroupedDefects = std::get<2>(ungroupedDefectsDumbbellPair);
      REQUIRE(ungroupedDefects.size() == 13 * 2);
      int nDefects;
      double inClusterFractionI, inClusterFractionV;
      std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
          csaransh::getNDefectsAndClusterFractions(ungroupedDefects);
      /*
      std::sort(std::begin(ungroupedDefects), std::end(ungroupedDefects), [](const auto &a, const auto &b) {
        if (std::get<0>(a)[0] == std::get<0>(b)[0]) return std::get<0>(a)[1] < std::get<0>(b)[1];
        return std::get<0>(a)[0] < std::get<0>(b)[0];
      });
      for (auto x : ungroupedDefects) {
        for (auto c : std::get<0>(x)) std::cout << c << ", ";
        std::cout << std::get<1>(x) << ", " << std::get<2>(x) << ", " << std::get<3>(x);
        std::cout << std::endl;
      }
      ungroupedDefects = {
{{{-168.547, 2.3739, -16.6173}}, 0, 1, 1}	,
{{{-166.964, -30.8607, -27.6955}}, 0, 2, 1}	,
{{{-163.799, -113.156, 19.7825}}, 0, 3, 1}	,
{{{-155.886, 2.3739, -32.4433}}, 0, 4, 0}	,
{{{-155.652, 1.48622, -33.0678}}, 1, 5, 0}	,
{{{-155.42, 3.22437, -31.5164}}, 1, 6, 0}	,
{{{-154.304, 0.7913, -34.0259}}, 0, 7, 0}	,
{{{-154.304, 3.9565, -30.8607}}, 0, 8, 0}	,
{{{-153.783, 1.57628, -34.8461}}, 1, 9, 0}	,
{{{-153.627, -0.374023, -33.1668}}, 1, 10, 0}	,
{{{-153.606, 5.09886, -31.6038}}, 1, 11, 1}	,
{{{-153.531, 3.27719, -29.6802}}, 1, 12, 1}	,
{{{-152.721, -0.7913, -32.4433}}, 0, 13, 0}	,
{{{-152.721, 2.3739, -29.2781}}, 0, 14, 0}	,
{{{-152.721, 2.3739, -35.6085}}, 0, 15, 0}	,
{{{-152.721, 5.5391, -32.4433}}, 0, 16, 0}	,
{{{-152.263, 1.98476, -36.8897}}, 1, 17, 0}	,
{{{-151.98, 3.48003, -34.8472}}, 1, 18, 0}	,
{{{-151.911, -0.262889, -31.6111}}, 1, 19, 0}	,
{{{-151.88, 1.59063, -29.7559}}, 1, 20, 0}	,
{{{-151.784, 5.40975, -33.1103}}, 1, 21, 0}	,
{{{-151.138, 0.7913, -30.8607}}, 0, 22, 0}	,
{{{-151.138, 3.9565, -34.0259}}, 0, 23, 0}	,
{{{-150.064, 1.60602, -31.6352}}, 1, 24, 0}	,
{{{-149.965, 3.26968, -33.2595}}, 1, 25, 1}	,
{{{-149.556, 2.3739, -32.4433}}, 0, 26, 0}	
      };
*/
      SECTION("Check cluster grouping") {
        auto defects = groupDefects(ungroupedDefects, latticeConst);
        auto clusterSizeMap = clusterSizes(defects);
        REQUIRE(clusterSizeMap.size() == 4);
        SECTION("Check ndefects and cluster sizes") {
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(nDefects == 3);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(100.0));
          ignoreSmallClusters(defects, clusterSizeMap);
          std::tie(nDefects, inClusterFractionI, inClusterFractionV) =
              csaransh::getNDefectsAndClusterFractions(defects);
          REQUIRE(inClusterFractionI == Approx(100.0));
          REQUIRE(inClusterFractionV == Approx(0.0));
          auto clusterIdMap = csaransh::clusterMapping(defects);
          REQUIRE(clusterIdMap.size() == 1); // 1 interstitial cluster ring
          auto it = std::begin(clusterIdMap);
          REQUIRE(it->second.size() == 23);
          auto clusterIVMap =
              csaransh::clusterIVType(clusterIdMap, clusterSizeMap);
          REQUIRE(clusterIVMap.size() == 1);
          REQUIRE(std::abs(std::begin(clusterIVMap)->second) == 3); // surviving
          int maxClusterSizeI, maxClusterSizeV;
          std::tie(maxClusterSizeI, maxClusterSizeV) =
              csaransh::getMaxClusterSizes(clusterSizeMap, clusterIdMap);
          REQUIRE(maxClusterSizeI == 3);
          REQUIRE(maxClusterSizeV == 0);
          SECTION("Check cluster features") {
            auto feats = csaransh::clusterFeatures(
                defects, clusterIdMap, clusterSizeMap, latticeConst);
            REQUIRE(feats.size() == 1);
            const auto &distFeat = std::get<0>(std::begin(feats)->second);
            REQUIRE(distFeat[0] == Approx(0.0));
            REQUIRE(std::accumulate(begin(distFeat), end(distFeat), 0.0) ==
                    Approx(1.0));
            const auto &angleFeat = std::get<1>(std::begin(feats)->second);
            // No zero or 180 degree angles in a ring
            REQUIRE(angleFeat[0] == Approx(0.0));
            REQUIRE(angleFeat[angleFeat.size() - 1] == Approx(0.0)); // 0.0
            // It should kind of rise in between but hard to pin the whole
            // structure down
            REQUIRE(std::accumulate(begin(angleFeat), end(angleFeat), 0.0) ==
                    Approx(1.0));
            const auto &adjFeat = std::get<2>(std::begin(feats)->second);
            REQUIRE(adjFeat[0] == Approx(0.0));
            REQUIRE(std::accumulate(begin(adjFeat), end(adjFeat), 0.0) ==
                    Approx(1.0));
          }
        } // ndefects and cluster sizes
      }   // cluster grouping
    }
  }
}
