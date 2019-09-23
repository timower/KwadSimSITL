#!/bin/bash
set -e

if [ "$TRAVIS_OS_NAME" = "osx" ]; then
    mkdir build
    cd build
    cmake ..
    cmake --build .
    test/unit_tests
    mv build/kwadSimServer build/kwadSimServer.osx
else
    # TODO: mingw windows build
    mkdir build
    cd build
    cmake ..
    cmake --build .
    test/unit_tests
    cd ..

    mkdir build_win
    cd build_win
    cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/windows.cmake ..
    cmake --build . --target kwadSimServer # TODO: resolve threads when using mingw from linux
    # TODO: fix wine64 test/unit_tests.exe
fi