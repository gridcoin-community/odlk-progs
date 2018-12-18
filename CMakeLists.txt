cmake_minimum_required(VERSION 3.5)

# Set the project name
project (odlk_mar)

# Add an executable
add_executable(family_mar
  source/prov_blk_main.cpp
  source/kanonizator.cpp
  source/prov_blk_trans.cpp)

#target_compile_features(family_mar PRIVATE cxx_std_11)
