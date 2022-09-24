cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project("engine")

add_library(${PROJECT_NAME} STATIC
	"include/engine/types.hpp"
	"include/engine/engine.hpp"
	"include/engine/resource.hpp"

	"src/engine/engine.cpp"
	"src/engine/application.cpp"

	"src/resource/texture_loader.cpp"
	"src/resource/mesh_loader.cpp"
)
target_include_directories(${PROJECT_NAME} PUBLIC "include")

add_subdirectory("external/libraries")
target_link_libraries(${PROJECT_NAME} PRIVATE libraries)



set(PROJECT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin/$<0:>")

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
