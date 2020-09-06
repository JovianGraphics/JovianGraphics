# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("JovianGames")

if(WIN32)
  set(CMAKE_CXX_FLAGS "/std:c++17 /WX /EHsc")
else()
  set(CMAKE_CXX_FLAGS "-std=c++17 -werror")
endif()

configure_file(config.h.in generated/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(External/glm)
include_directories(${CMAKE_SOURCE_DIR}/External/glm)

add_subdirectory(External/preprocessor)
include_directories(${CMAKE_SOURCE_DIR}/External/preprocessor/include)
include_directories(${CMAKE_SOURCE_DIR}/External/imgui)
include_directories(${CMAKE_SOURCE_DIR}/External/implot)

add_subdirectory(Ganymede)
add_subdirectory(Himalia)
add_subdirectory(Io)
add_subdirectory(Europa)
add_subdirectory(Amalthea)
