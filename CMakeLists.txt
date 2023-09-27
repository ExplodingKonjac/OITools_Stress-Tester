cmake_minimum_required(VERSION 3.26)
project("OITools::Stress-Tester")

# settings
set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED true)
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
	message("Release mode.")
	add_compile_options(-O3 -Wall -DNDEBUG)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Build/Release/)
elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
	message("Debug mode.")
	add_compile_options(-g -Wall)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/Build/Debug/)
else()
	message(FATAL_ERROR "Unknown build type.")
endif()

# requires
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	execute_process(COMMAND zenity --version RESULT_VARIABLE res)
	if(NOT ${res} EQUAL 0)
		message(FATAL_ERROR "Requires zenity. Use 'sudo apt install zenity' to install.")
	endif()
endif()
find_package(Boost COMPONENTS filesystem REQUIRED)

# checkers
set(checkers_exe_list)
add_subdirectory(checkers/)

# main
aux_source_directory(src/core/ core_src_list)
aux_source_directory(src/base/ base_src_list)
include_directories(src/base/)
add_executable(oit-stress src/main.cpp ${core_src_list} ${base_src_list})
target_link_libraries(oit-stress ${Boost_LIBRARIES})
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_link_libraries(oit-stress wsock32 ws2_32)
endif()

# installing
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_INSTALL_PREFIX "C:/Program Files/OITools/")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(CMAKE_INSTALL_PREFIX "/usr/local/")
endif()
install(TARGETS oit-stress ${checkers_exe_list} RUNTIME DESTINATION bin/)
