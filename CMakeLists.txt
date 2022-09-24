cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project("game")

add_executable(${PROJECT_NAME}
	"src/main.cpp"
	"src/helper.hpp"
)

add_subdirectory("engine")
target_link_libraries(${PROJECT_NAME} engine)



set(PROJECT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/$<0:>")

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "${PROJECT_BINARY_DIR}/shaders"
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" "${PROJECT_BINARY_DIR}/resources"
)
