cmake_minimum_required(VERSION 3.16)
project(rutas_cafetero CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# Misma bandera para todas las mediciones (se reporta en el documento tecnico)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(dijkstra cpp/dijkstra.cpp)
add_executable(hull     cpp/hull.cpp)
add_executable(tsp      cpp/tsp.cpp)
