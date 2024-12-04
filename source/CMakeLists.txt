cmake_minimum_required(VERSION 3.11)
project(2024_PP_Final_Group11)
set(CMAKE_CXX_STANDARD 14)

# OpenMP
find_package(OpenMP REQUIRED)

# MPI
set(MSMPI_INC "C:/Program Files/Microsoft MPI/Inc")
set(MSMPI_LIB "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")
include_directories(${MSMPI_INC})

add_subdirectory("src")
add_subdirectory("extern")
