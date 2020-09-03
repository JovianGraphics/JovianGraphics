# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Io")

set(IoSource Source/IoSurfaceWin32.cpp)

add_library(Io ${IoSource})

set_property(TARGET Io PROPERTY CXX_STANDARD 17)

if(WIN32)

add_executable(IoTest WIN32
	Test/test.cpp
)

else()

add_executable(IoTest
	Test/test.cpp
)

endif()

target_link_libraries(IoTest PUBLIC Io)

include_directories(Source)