# 2024-NYCU-PP-Final

Authors: 110550170 戚維凌、110550034 孫承瑞

## Setup
1. `git clone https://github.com/Rebeccasun31/2024-NYCU-PP-Final.git`

2. `git submodule update --init --recursive`

3. Modify the options

4. Build

5. Run
    - Serial: `"build/src/2024_PP_Final_Group11_omp.exe"`

    - OpenMP: `"build/src/2024_PP_Final_Group11_omp.exe" <number of threads>`

    - MPI: `mpiexec -n <number of processes> "build/src/2024_PP_Final_Group11_mpi.exe"`

    - CUDA: `"build/src/2024_PP_Final_Group11_omp.exe"`

## Options
1. Parallel Method:

    In `src/header/nbody.h`, uncomment the method you want.

2. Number of Particles:

    In `src/header/point.h`, modify the POINT_CNT parameter.
