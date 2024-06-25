add_library(glad ${CMAKE_CURRENT_SOURCE_DIR}/glad.c)

target_include_directories(glad 
		PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_CURRENT_SOURCE_DIR}/KHR"
	)

