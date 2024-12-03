add_executable(ICG_2024_HW2
"main.cpp"
"stb_image.cpp"
"point.cpp"
"nbody_serial.cpp"
"nbody_omp.cpp"
"camera.cpp"
) #列所有的cpp

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ICG_2024_HW2
    PUBLIC
    glfw
    glm::glm
    glad
    OpenMP::OpenMP_CXX
    )
endif()
