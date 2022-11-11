cmake_minimum_required(VERSION 3.0)

project(preproc)

set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 14)

add_executable(preproc main.cpp)