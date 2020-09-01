# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Himalia")

add_library(Himalia
	Source/Himalia.cpp
)

set_property(TARGET Himalia PROPERTY CXX_STANDARD 17)

target_link_libraries(Himalia PUBLIC Ganymede)

include_directories(Source)