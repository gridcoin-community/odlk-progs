cmake_minimum_required(VERSION 3.5)

LIST(APPEND CMAKE_MODULE_PATH cmakefbc/cmake/Modules)

# Set the project name
project (odlk C CXX Fbc)

# Paralelism
find_package( Threads )

#Set the exe suffix. Otherwise the binary will collide
# with a directory of the same name.
if("${CMAKE_EXECUTABLE_SUFFIX}" STREQUAL "")
	set(CMAKE_EXECUTABLE_SUFFIX ".exe")
endif()

# Find portable-snippets library, for _BitScanForward intristics
find_file( PSNIP_BUILTIN_H "psnip/builtin/builtin.h" PATHS "." NO_DEFAULT_PATH)
if(NOT PSNIP_BUILTIN_H)
    # How can I exit cmake with an error message if bzlib.h does not exists?
    message( FATAL_ERROR "Library psnip not found. Please initialize git "
      "submodules or download portable snipets into psnip/ directory from "
      "https://github.com/nemequ/portable-snippets" )
endif()

# The family_mar program

add_executable(family_mar
  odlkcommon/kvio.cpp
  kanonizator_dlk/kanonizator.cpp
  kanonizator_dlk/hash_tabl.cpp
  family_mar/prov_blk_main.cpp
  family_mar/prov_blk_trans.cpp)

target_include_directories(family_mar PRIVATE ".")

# Paralelism
target_compile_features(family_mar PRIVATE cxx_std_11)
target_compile_definitions(family_mar PRIVATE USE_THREADS)
target_link_libraries(family_mar ${CMAKE_THREAD_LIBS_INIT})

# gen_lk_4_31_31

add_executable(gen_lk_4_31_31
  odlkcommon/kvio.cpp
  gen_lk_4_31_31/gen_lk_4_31_31.cpp
  gen_lk_4_31_31/izomorfizm.cpp
  gen_lk_4_31_31/kanon.cpp
  gen_lk_4_31_31/kan_shab_4_31_31.cpp)

target_include_directories(gen_lk_4_31_31 PRIVATE ".")

# The kanonizator_lk_po_dlk program

add_executable(kanonizator_dlk
  odlkcommon/kvio.cpp
  kanonizator_dlk/kanonizator.cpp
  kanonizator_dlk/hash_tabl.cpp
  kanonizator_dlk/main_kanon.cpp)

target_include_directories(kanonizator_dlk PRIVATE ".")

# The kanonizator_lk program

add_executable(kanonizator_lk
  odlkcommon/kvio.cpp
  kanonizator_lk/kanon.cpp
  kanonizator_lk/main.cpp)

target_include_directories(kanonizator_lk PRIVATE ".")

# PADS Generators

SET(CMAKE_Fbc_FLAGS "${CMAKE_Fbc_FLAGS} -lang deprecated")
SET(CMAKE_Fbc_FLAGS_DEBUG "${CMAKE_Fbc_FLAGS} -lang deprecated -g")

add_executable(psevdoass
  psevdoass/ASS_DLK10A.BAS
  psevdoass/PSEVDOASS_DLK_new.BAS
  kanonizator_dlk_1.03/kanonizator.cpp
  kanonizator_dlk_1.03/hash_tabl.cpp
  family_mar/prov_blk_trans.cpp
  psevdoass/psevdoass.cpp)

target_include_directories(psevdoass PRIVATE ".")
# Paralelism
target_compile_features(psevdoass PRIVATE cxx_std_11)
target_compile_definitions(psevdoass PRIVATE USE_THREADS)
target_link_libraries(psevdoass ${CMAKE_THREAD_LIBS_INIT})
