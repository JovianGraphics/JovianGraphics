# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Europa")

add_library(Amalthea
	Source/Amalthea.cpp
)

target_link_libraries(Amalthea PUBLIC Europa Ganymede)

add_custom_command(
	OUTPUT unlit.frag.h
	PRE_BUILD
	COMMAND ruby ${JovianSrcDir}/Europa/Tools/compile_shader.rb ${JovianSrcDir}/Amalthea/Test/unlit.frag output=${CMAKE_BINARY_DIR}/generated/unlit.frag.h
	MAIN_DEPENDENCY ${JovianSrcDir}/Amalthea/Test/unlit.frag
)

add_custom_command(
	OUTPUT unlit.vert.h
	PRE_BUILD
	COMMAND ruby ${JovianSrcDir}/Europa/Tools/compile_shader.rb ${JovianSrcDir}/Amalthea/Test/unlit.vert output=${CMAKE_BINARY_DIR}/generated/unlit.vert.h
	MAIN_DEPENDENCY ${JovianSrcDir}/Amalthea/Test/unlit.vert
)

add_custom_target(AmaltheaTestShaders ALL DEPENDS
	unlit.frag.h
	unlit.vert.h
)

if(WIN32)

add_executable(AmaltheaTest WIN32
	Test/test.cpp
)
target_sources(AmaltheaTest PRIVATE ${JovianSrcDir}/Io/WindowsHiDPI.manifest)
set_property(TARGET AmaltheaTest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${JovianSrcDir}")

else()

add_executable(AmaltheaTest
	Test/test.cpp
)

endif()

set_property(TARGET AmaltheaTest PROPERTY CXX_STANDARD 17)

target_link_libraries(AmaltheaTest PUBLIC Amalthea Europa Io Himalia)
add_dependencies(AmaltheaTest AmaltheaTestShaders)

include_directories(Source)