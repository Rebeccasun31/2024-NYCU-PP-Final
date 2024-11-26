add_executable(ICG_2024_HW2
"main.cpp"
"stb_image.cpp"
"point.cpp"
"nbody_serial.cpp"
"camera.cpp"
) #列所有的cpp
target_link_libraries(ICG_2024_HW2
glfw
glm::glm
glad
)