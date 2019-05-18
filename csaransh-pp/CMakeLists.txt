cmake_minimum_required(VERSION 2.8)
project(csaransh-pp)

set (csaransh_VERSION_MAJOR 0)
set (csaransh_VERSION_MINOR 3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_COMPILER_IS_GNUCC)
  option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

  if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0)
  endif()
endif()

include_directories(include third_party)

set(SOURCES src/AddOffset.cpp src/cluster2features.cpp src/printJson.cpp src/reader.cpp src/results.cpp src/xyz2defects.cpp)
file(GLOB TESTS test/*.cpp)

add_library(${PROJECT_NAME}_lib ${SOURCES})
add_executable(${PROJECT_NAME} src/main.cpp)
add_executable(${PROJECT_NAME}_test ${TESTS})

target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib --coverage)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME}_lib --coverage)

include(CTest)

add_test(NAME ${PROJECT_NAME}_test
          COMMAND "${CMAKE_BINARY_DIR}/${PROJECT_NAME}_test"
          WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS ${PROJECT_NAME}_test)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
