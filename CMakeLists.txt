# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("JovianGames")

configure_file(config.h.in generated/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(External/glm)
include_directories(${CMAKE_SOURCE_DIR}/External/glm)

add_subdirectory(External/preprocessor)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(Ganymede)
add_subdirectory(Io)
add_subdirectory(Europa)
