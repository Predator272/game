cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project("libraries")

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE "include")
target_compile_definitions(${PROJECT_NAME} INTERFACE STB_IMAGE_IMPLEMENTATION)
target_compile_definitions(${PROJECT_NAME} INTERFACE TINYOBJLOADER_IMPLEMENTATION)
