add_library(utils 
	${CMAKE_CURRENT_SOURCE_DIR}/utils/window.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/utils/shader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/camera.cpp)

find_package(glfw3 3.3 REQUIRED)

target_include_directories(utils PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src/utils ${PROJECT_SOURCE_DIR}/include/stb) 

target_link_libraries(utils glfw)
