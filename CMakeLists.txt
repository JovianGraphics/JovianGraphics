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

set(JovianSrcDir ${CMAKE_CURRENT_SOURCE_DIR})

configure_file(config.h.in generated/config.h)

add_subdirectory(External/glm)
add_subdirectory(External/preprocessor)

set(JovianIncludeDir
  ${CMAKE_CURRENT_BINARY_DIR}/generated
  ${JovianSrcDir}
  ${JovianSrcDir}/External/glm
  ${JovianSrcDir}/External/preprocessor/include
  ${JovianSrcDir}/External/imgui
  ${JovianSrcDir}/External/implot
)

include_directories(${JovianIncludeDir})

set(JovianIncludeDir ${JovianIncludeDir} CACHE STRING "Jovian Graphics Include Directories")

add_subdirectory(Ganymede)
add_subdirectory(Himalia)
add_subdirectory(Io)
add_subdirectory(Europa)
add_subdirectory(Amalthea)
