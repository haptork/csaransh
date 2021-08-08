#include <catch.hpp>

#include <helper.hpp>

using namespace csaransh;
TEST_CASE("distance calculation tests", "[calcDistTest]") {
  auto origin = Coords{{0, 0, 0}};
  auto a = Coords{{1.5, 0, 0}};
  auto b = Coords{{0, 2.5, 0}};
  auto c = Coords{{0, 0, 3.0}};
  SECTION("Common Cases") {
    auto root3 = std::sqrt(3.0);
    auto d = Coords{{root3, root3, root3}};
    REQUIRE(calcDist(origin, d) == Approx(3.0));
    REQUIRE(calcDist(Coords{{2.0, 4.0, 1.0}}, Coords{{4.0, 4.0, 1.0}}) ==
            Approx(2.0));
    REQUIRE(calcDist(a, d) == Approx(2.46045));
    REQUIRE(calcDist(b, c) == Approx(3.90512));
  }
  SECTION("Edge Cases - 1 axis change") {
    REQUIRE(calcDist(origin, a) == Approx(1.5));
    REQUIRE(calcDist(origin, b) == Approx(2.5));
    REQUIRE(calcDist(origin, c) == Approx(3.0));
  }
  SECTION("Edge Case: zero") {
    REQUIRE(calcDist(origin, origin) == Approx(0));
    REQUIRE(calcDist(a, a) == Approx(0));
    REQUIRE(calcDist(b, Coords{{0, 2.5, 0}}) == Approx(0));
    REQUIRE(calcDist(c, c) == Approx(calcDist(b, b)));
  }
}
TEST_CASE("Tests for trims") {
  std::string a1{"hey"};
  std::string b1{"   hey"};
  std::string c1{"hey  "};
  std::string d1{"   hey  "};
  std::string a2{"hello world"};
  std::string b2{"   hello world"};
  std::string c2{"hello world "};
  std::string d2{" hello world "};
  std::string blank{""};
  SECTION("Normal Cases ltrim") {
    ltrim(b1);
    REQUIRE(b1 == a1);
    ltrim(d1);
    REQUIRE(d1 == c1);
    ltrim(b2);
    REQUIRE(b2 == a2);
    auto c2copy = c2;
    ltrim(c2);
    REQUIRE(c2 == c2copy);
    ltrim(d2);
    REQUIRE(d2 == c2);
  }
  SECTION("Edge Cases ltrim") {
    auto a1copy = a1;
    ltrim(a1);
    REQUIRE(a1 == a1copy);
    auto a2copy = a2;
    ltrim(a2);
    REQUIRE(a2 == a2copy);
    ltrim(blank);
    REQUIRE(blank == blank);
  }
  SECTION("Normal Cases rtrim") {
    auto b1copy = b1;
    rtrim(b1);
    REQUIRE(b1 == b1copy);
    rtrim(c1);
    REQUIRE(c1 == a1);
    rtrim(d1);
    REQUIRE(d1 == b1);
    rtrim(c2);
    REQUIRE(c2 == a2);
  }
  SECTION("Edge Cases rtrim") {
    auto a1copy = a1;
    rtrim(a1);
    REQUIRE(a1 == a1copy);
    auto a2copy = a2;
    rtrim(a2);
    REQUIRE(a2 == a2copy);
    rtrim(blank);
    REQUIRE(blank == blank);
  }
  SECTION("Normal Cases trim") {
    REQUIRE(trim(b1) == a1);
    REQUIRE(trim(c1) == a1);
    REQUIRE(trim(d1) == a1);
    REQUIRE(trim(b2) == a2);
    REQUIRE(trim(c2) == a2);
    REQUIRE(trim(d2) == a2);
  }
  SECTION("Edge Cases trim") {
    auto a1copy = a1;
    REQUIRE(trim(a1) == a1copy);
    auto a2copy = a2;
    REQUIRE(trim(a2) == a2copy);
    REQUIRE(trim(blank) == blank);
  }
}
TEST_CASE("Tests for replaceStr") {
  std::string replace{"hello"};
  std::string from{"--- hello world hello --- "};
  std::string to{"hey"};
  auto res1 = replaceStr(from, replace, to);
  REQUIRE(from == std::string{"--- hey world hello --- "});
  REQUIRE(res1);
  auto res2 = replaceStr(from, replace, to);
  REQUIRE(from == std::string{"--- hey world hey --- "});
  REQUIRE(res2);
  auto res3 = replaceStr(from, replace, to);
  REQUIRE(from == std::string{"--- hey world hey --- "});
  REQUIRE_FALSE(res3);
}
TEST_CASE("Tests for remove parcas comments") {
  std::string a{"hello world hello --- "};
  std::string b{"=3.165 lattice constant of W"};
  std::string c{"  1.000    -- multiplicative identity"};
  std::string d{"= 0.000    -- additive identity"};
  REQUIRE(removeParcasComments(a) == "hello");
  REQUIRE(removeParcasComments(b) == "=3.165");
  REQUIRE(removeParcasComments(c) == "1.000");
  REQUIRE(removeParcasComments(d) == "=");
}
