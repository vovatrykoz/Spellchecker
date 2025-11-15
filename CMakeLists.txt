cmake_minimum_required(VERSION 3.20)
project(Simple_Spellchecker)

function(enable_maximum_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /sdl)
        # target_compile_options(${target} PRIVATE /Wall)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target} PRIVATE
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-padded
            -Werror
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wdouble-promotion
            -Wformat=2
            -Werror
        )
    endif()
endfunction()

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(Spellchecker_lib "${PROJECT_SOURCE_DIR}/source/spellchecker.cpp")

add_executable(Spellchecker "${PROJECT_SOURCE_DIR}/source/main.cpp")
enable_maximum_warnings(Spellchecker)

target_link_libraries(Spellchecker Spellchecker_lib)

set_property(TARGET Spellchecker Spellchecker_lib PROPERTY CXX_STANDARD 20)
set_property(TARGET Spellchecker Spellchecker_lib PROPERTY CXX_STANDARD_REQUIRED On)

add_custom_target(run Spellchecker)