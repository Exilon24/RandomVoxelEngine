cmake_minimum_required(VERSION 3.28)
project(TestImgui)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package(glfw3 3.3 REQUIRED)

add_executable(MainExec src/main.cpp)

add_subdirectory(include/glm)
add_subdirectory(include/glad)
add_subdirectory(src)

target_link_libraries(MainExec utils glfw glad glm /opt/homebrew/Cellar/assimp/5.3.1/lib/libassimp.dylib)

target_include_directories(MainExec PUBLIC 
	${PROJECT_SOURCE_DIR}/include 
	"${PROJECT_SOURCE_DIR}" 
	"${PROJECT_SOURCE_DIR}include/stb" 
	"${PROJECT_SOURCE_DIR}/src/utils"
	"${PROJECT_SOURCE_DIR}/assimp")
