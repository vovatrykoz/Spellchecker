cmake_minimum_required(VERSION 3.20)
project(Simple_Spellchecker)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(Spellchecker_lib "${PROJECT_SOURCE_DIR}/source/spellchecker.cpp")

add_executable(Spellchecker "${PROJECT_SOURCE_DIR}/source/main.cpp")

target_link_libraries(Spellchecker Spellchecker_lib)

set_property(TARGET Spellchecker Spellchecker_lib PROPERTY CXX_STANDARD 20)
set_property(TARGET Spellchecker Spellchecker_lib PROPERTY CXX_STANDARD_REQUIRED On)

add_custom_target(run Spellchecker)