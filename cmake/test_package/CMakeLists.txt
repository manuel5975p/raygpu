# Consumer smoke test for the installed raygpu package.
# Used by CI to verify a release archive links before it is uploaded:
#   cmake -S cmake/test_package -B build -DCMAKE_PREFIX_PATH=<install prefix>
cmake_minimum_required(VERSION 3.25)
project(raygpu_test_package C)

find_package(raygpu CONFIG REQUIRED)

add_executable(test_package main.c)
target_link_libraries(test_package PRIVATE raygpu::raygpu)
