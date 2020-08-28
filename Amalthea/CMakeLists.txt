# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Europa")

add_library(Amalthea
	Source/Amalthea.cpp
	Source/AmaltheaPipeline.cpp
)

set_property(TARGET Amalthea PROPERTY CXX_STANDARD 17)

target_link_libraries(Amalthea PUBLIC Europa Ganymede)

add_custom_command(
	OUTPUT unlit.frag.h
	PRE_BUILD
	COMMAND ruby ${CMAKE_SOURCE_DIR}/Europa/Tools/compile_shader.rb ${CMAKE_SOURCE_DIR}/Amalthea/Test/unlit.frag output=${CMAKE_BINARY_DIR}/generated/unlit.frag.h
)

add_custom_command(
	OUTPUT unlit.vert.h
	PRE_BUILD
	COMMAND ruby ${CMAKE_SOURCE_DIR}/Europa/Tools/compile_shader.rb ${CMAKE_SOURCE_DIR}/Amalthea/Test/unlit.vert output=${CMAKE_BINARY_DIR}/generated/unlit.vert.h
)

add_custom_target(AmaltheaTestShaders ALL DEPENDS
	unlit.frag.h
	unlit.vert.h
)

if(WIN32)

add_executable(AmaltheaTest WIN32
	Test/test.cpp
)

else()

add_executable(AmaltheaTest
	Test/test.cpp
)

endif()

target_link_libraries(AmaltheaTest PUBLIC Amalthea Europa Io)
add_dependencies(AmaltheaTest AmaltheaTestShaders)

include_directories(Source)