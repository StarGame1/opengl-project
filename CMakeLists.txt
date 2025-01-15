cmake_minimum_required(VERSION 3.17)
project(untitled3)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(glfw3 REQUIRED)

# Add source files
set(SOURCE_FILES
        main.cpp
        Shader.cpp
        Camera.cpp
        Model3D.cpp
        Mesh.cpp
        tiny_obj_loader.cpp
        stb_image.cpp
        Window.cpp
)


# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link against GLFW
target_link_libraries(${PROJECT_NAME} glfw)

# Mac-specific configuration
if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})