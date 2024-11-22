cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(zfw2)

add_subdirectory(zfw2)
add_subdirectory(zfw2_asset_packer)
add_subdirectory(zfw2_common)
add_subdirectory(sandbox)
