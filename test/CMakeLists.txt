
add_executable(unit_tests test.cpp test_vmath.cpp test_packets.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_include_directories(unit_tests PRIVATE catch/)
target_include_directories(unit_tests PRIVATE ../src/)

target_link_libraries(unit_tests PUBLIC libsim)

target_link_libraries(unit_tests PUBLIC Threads::Threads)

target_link_libraries(unit_tests PRIVATE fmt-header-only)

if (WIN32)
    target_link_libraries(unit_tests PUBLIC wsock32 ws2_32)
endif (WIN32)

enable_testing()
add_test(NAME kwadSimSITLTests COMMAND unit_tests)
