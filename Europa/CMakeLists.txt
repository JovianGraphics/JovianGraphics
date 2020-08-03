# CMakeList.txt : CMake project for JovianGames, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Europa")

add_library(Europa
	Source/EuropaVk.cpp
)

set_property(TARGET Europa PROPERTY CXX_STANDARD 14)

find_package(Vulkan REQUIRED)
target_link_libraries(Europa PUBLIC Ganymede Vulkan::Vulkan)
include_directories(PUBLIC ${Vulkan_INCLUDE_DIRS})

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

include_directories(Source)