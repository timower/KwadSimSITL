cmake_minimum_required(VERSION 3.3)
project(KwadSimServer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linker options for betaflight and windows
if (NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/external/betaflight/src/main/target/SITL/pg.ld")
endif()

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")


add_subdirectory(external/fmt EXCLUDE_FROM_ALL)

# Get betaflight sources:
add_subdirectory(external/)

set(SOURCE_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulator.cpp)

add_library(libsim OBJECT ${BETAFLIGHT_SOURCES} ${SOURCE_FILES})
target_compile_features(libsim PUBLIC cxx_std_17)

target_compile_definitions(libsim PUBLIC "__TARGET__=\"KWADSIM\"")
target_compile_definitions(libsim PUBLIC "__REVISION__=\"TODO\"")

target_compile_definitions(libsim PUBLIC SIMULATOR_BUILD)
target_compile_definitions(libsim PUBLIC "HSE_VALUE=8000000")
target_compile_definitions(libsim PUBLIC "FLASH_SIZE=2048")


target_include_directories(libsim PUBLIC external)
target_include_directories(libsim PUBLIC external/src/)
target_include_directories(libsim PUBLIC external/betaflight/lib/main/dyad)
target_include_directories(libsim PUBLIC external/betaflight/src/main)

target_include_directories(libsim PUBLIC external/kissnet)

target_link_libraries(libsim PUBLIC fmt-header-only)

add_executable(kwadSimServer 
    src/main.cpp $<TARGET_OBJECTS:libsim>) # ${SOURCE_FILES}) #${BETAFLIGHT_SOURCES})

target_link_libraries(kwadSimServer PUBLIC libsim)

if (WIN32)
    target_link_libraries(kwadSimServer PUBLIC wsock32 ws2_32)
endif (WIN32)

add_subdirectory(test)
