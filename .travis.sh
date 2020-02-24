#!/bin/bash
set -e

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .
    test/unit_tests
    mv kwadSimSITL kwadSimSITL.osx
else
    # build tests
    mkdir build
    cd build
    cmake -DGEN_COVERAGE=ON ..
    cmake --build .
    test/unit_tests
    cd ..

    # build release, but do run unit tests in order to dected float error differences
    mkdir build_rel
    cd build_rel
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .
    test/unit_tests
    mv kwadSimSITL kwadSimSITL.x11
    cd ..

    mkdir build_win
    cd build_win
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/windows.cmake ..
    cmake --build . --target kwadSimSITL 
    # TODO: resolve threads when using mingw from linux
    # TODO: fix wine64 test/unit_tests.exe
fi
