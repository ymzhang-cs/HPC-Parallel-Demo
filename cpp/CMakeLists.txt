cmake_minimum_required(VERSION 3.14)
project(image_processing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/usr/local/lib/cmake/pybind11")
find_package(pybind11 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)

pybind11_add_module(image_processing image_processing.cpp)
target_link_libraries(image_processing PRIVATE OpenMP::OpenMP_CXX ${OpenCV_LIBS})
