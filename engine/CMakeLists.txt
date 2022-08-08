cmake_minimum_required(VERSION 3.23.2)
project(DotEngine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(SRC 
	src/DotEngine.cpp
    src/Graphics.cpp
    src/Window.cpp
	src/DotException.cpp
	src/Shader.cpp
)

find_package(Vulkan REQUIRED)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

add_library(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        glm
        glfw
    PRIVATE
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        glm
        glfw
    PRIVATE
        ${Vulkan_LIBRARIES}
)

