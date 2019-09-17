#!/bin/bash

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    mkdir build
    cd build
    cmake ..
    cmake --build .
    test/unit_tests
else
    # TODO: mingw windows build
    mkdir build
    cd build
    cmake ..
    cmake --build .
    test/unit_tests
fi