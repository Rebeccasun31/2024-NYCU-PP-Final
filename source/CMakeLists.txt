cmake_minimum_required(VERSION 3.11)
# project(2024_PP_Final_Group11 LANGUAGES CXX)
project(2024_PP_Final_Group11 LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 14)

# OpenMP
find_package(OpenMP REQUIRED)

# MPI
set(MSMPI_INC "C:/Program Files/Microsoft MPI/Inc")
set(MSMPI_LIB "C:/Program Files (x86)/Microsoft SDKs/MPI/Lib/x64")
include_directories(${MSMPI_INC})

# CUDA
enable_language(CUDA)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -O3")

target_compile_options(2024_PP_Final_Group11 PRIVATE 
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
)
set_target_properties(2024_PP_Final_Group11 PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

add_subdirectory("src")
add_subdirectory("extern")