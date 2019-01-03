cmake_minimum_required(VERSION 3.5)

# Set the project name
project (odlk)

if("${CMAKE_EXECUTABLE_SUFFIX}" STREQUAL "")
	set(CMAKE_EXECUTABLE_SUFFIX ".exe")
endif()

# Add an executable
add_executable(family_mar
  family_mar/prov_blk_main.cpp
  family_mar/kanonizator.cpp
  family_mar/prov_blk_trans.cpp)

#target_compile_features(family_mar/family_mar PRIVATE cxx_std_11)
