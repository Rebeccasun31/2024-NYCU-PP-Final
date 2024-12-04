cmake_minimum_required(VERSION 3.11)
project(2024_PP_Final_Group11 LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 14)

# OpenMP
find_package(OpenMP REQUIRED)

# MPI
set(MSMPI_INC "C:/Program Files/Microsoft MPI/Inc")
set(MSMPI_LIB "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")
include_directories(${MSMPI_INC})

# CUDA
set(CMAKE_CUDA_HOST_COMPILER "C:/w64devkit/bin/g++.exe")
enable_language(CUDA)
set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3")
target_compile_options(2024_PP_Final_Group11 PRIVATE ${CUDA_NVCC_FLAGS})

add_subdirectory("src")
add_subdirectory("extern")