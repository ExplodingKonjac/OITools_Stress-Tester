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
if(WIN32)
	set(BOOST_ROOT D:/Libraries/boost/)
	set(Boost_NO_SYSTEM_PATHS ON)
endif()
find_package(Boost COMPONENTS filesystem REQUIRED)

# checkers
add_subdirectory(checkers/ EXCLUDE_FROM_ALL)

# main
aux_source_directory(src/core/ core_src_list)
aux_source_directory(src/base/ base_src_list)
include_directories(src/base/)
add_executable(stress src/main.cpp ${core_src_list} ${base_src_list})
target_link_libraries(stress ${Boost_LIBRARIES})
if(WIN32)
	target_link_libraries(stress wsock32 ws2_32)
endif()

# installing
install(TARGETS stress DESTINATION .)
