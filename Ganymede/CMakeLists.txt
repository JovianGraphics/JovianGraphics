# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Ganymede")

add_library(Ganymede
	Source/Ganymede.cpp
 "Source/Ganymede.h" )

include_directories(Source)