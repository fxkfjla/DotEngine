cmake_minimum_required(VERSION 3.23.2)
project(App VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(SRC 
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} DotEngine)
target_include_directories(${PROJECT_NAME} 
    PUBLIC
    PRIVATE
        DotEngine
)
