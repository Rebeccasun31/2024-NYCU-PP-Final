add_executable(2024_PP_Final_Group11
"main.cpp"
"stb_image.cpp"
"point.cpp"
"nbody_serial.cpp"
"nbody_omp.cpp"
"nbody_mpi.cpp"
"camera.cpp"
) #列所有的cpp

target_link_libraries(2024_PP_Final_Group11
    PUBLIC
    OpenMP::OpenMP_CXX
    "${MSMPI_LIB}/msmpi.lib"
    glfw
    glm::glm
    glad
)