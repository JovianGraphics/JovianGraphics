# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Europa")

set(EuropaSource
	Source/EuropaVk.cpp
	Source/EuropaVkAllocator.cpp
	Source/EuropaUtils.cpp
	Source/EuropaImGui.cpp

	${JovianSrcDir}/External/imgui/imgui.cpp
	${JovianSrcDir}/External/imgui/imgui_draw.cpp
	${JovianSrcDir}/External/imgui/imgui_widgets.cpp

	${JovianSrcDir}/External/implot/implot.cpp
	${JovianSrcDir}/External/implot/implot_items.cpp
)

if(WIN32)
	list(APPEND EuropaSource ${JovianSrcDir}/External/imgui/examples/imgui_impl_win32.cpp)
endif()

list(APPEND EuropaSource ${JovianSrcDir}/External/imgui/examples/imgui_impl_vulkan.cpp)

add_library(Europa ${EuropaSource})

find_package(Vulkan REQUIRED)
target_link_libraries(Europa PUBLIC Ganymede Vulkan::Vulkan)
include_directories(PUBLIC ${Vulkan_INCLUDE_DIRS})

add_custom_command(
	OUTPUT triangle.frag.h
	PRE_BUILD
	COMMAND ruby ${JovianSrcDir}/Europa/Tools/compile_shader.rb ${JovianSrcDir}/Europa/Test/triangle.frag output=${CMAKE_BINARY_DIR}/generated/triangle.frag.h
)

add_custom_command(
	OUTPUT triangle.vert.h
	PRE_BUILD
	COMMAND ruby ${JovianSrcDir}/Europa/Tools/compile_shader.rb ${JovianSrcDir}/Europa/Test/triangle.vert output=${CMAKE_BINARY_DIR}/generated/triangle.vert.h
)

add_custom_target(EuropaTestShaders ALL DEPENDS
	triangle.frag.h
	triangle.vert.h
)

if(WIN32)

add_executable(EuropaTest WIN32
	Test/test.cpp
)

else()

add_executable(EuropaTest
	Test/test.cpp
)

endif()

target_link_libraries(EuropaTest PUBLIC Europa Io)
add_dependencies(EuropaTest EuropaTestShaders)

include_directories(Source)