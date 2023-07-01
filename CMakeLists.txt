cmake_minimum_required(VERSION 3.26)
project("Stress Tester")

# settings
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED true)
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
	message("Release mode.")
	add_compile_options(-O2 -Wall -DNDEBUG)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Build/Release/)
elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
	message("Debug mode.")
	add_compile_options(-g -Wall)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Build/Debug/)
else()
	message(FATAL_ERROR "Unknown build type.")
endif()

# checkers
add_subdirectory(checkers/ EXCLUDE_FROM_ALL)

# main
aux_source_directory(src/core/ core_src_list)
include_directories(src/include/)
add_executable(stress src/main.cpp ${core_src_list})

# installing
install(TARGETS stress DESTINATION .)