# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Ganymede")

add_library(Ganymede
	Source/Ganymede.cpp
	Source/GanymedeECS.cpp
)

target_link_libraries(Ganymede Boost::preprocessor)

include_directories(Source)