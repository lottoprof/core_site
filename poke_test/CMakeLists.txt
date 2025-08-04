cmake_minimum_required (VERSION 2.8)
project (poke_test)
set (poke_test_version_major 0)
set (poke_test_version_minor 1)
message("--------------------------")
message("poke_test ${poke_test_version_major}.${poke_test_version_minor} ")
message("--------------------------")

set (CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_FLAGS " -std=c++14 -g")
add_executable(poke poke_test.cpp Poke/PokeHelper.cpp)
target_link_libraries(poke
	-lpthread
	-ldl
	-lm
)


#********************************
