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

# Add an executable
add_executable(gen_lk_4_31_31
  gen_lk_4_31_31/gen_lk_4_31_31.cpp
  gen_lk_4_31_31/izomorfizm.cpp
  gen_lk_4_31_31/kanon.cpp
  gen_lk_4_31_31/kan_shab_4_31_31.cpp)

target_include_directories(gen_lk_4_31_31 PRIVATE ".")
