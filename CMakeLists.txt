cmake_minimum_required(VERSION 3.20)
project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to an optimized build: a chess engine in Debug is far too slow to play.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Everything except main.cpp is a library, so the engine and the tests share it.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM ENGINE_SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)

add_library(engine STATIC ${ENGINE_SOURCES})
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR}/src)

if(MSVC)
  target_compile_options(engine PUBLIC /W3 /permissive-)
else()
  target_compile_options(engine PUBLIC -Wall)
endif()

add_executable(ChessEngine src/main.cpp)
target_link_libraries(ChessEngine PRIVATE engine)

# Move-generation test: run with `ctest` or the perft executable directly.
enable_testing()
add_executable(perft tests/perft.cpp)
target_link_libraries(perft PRIVATE engine)
add_test(NAME perft COMMAND perft)
