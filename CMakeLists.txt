cmake_minimum_required(VERSION 3.26)
project("OITools::Stress-Tester" VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# requirements
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	find_program(ZENITY_FOUND
		NAMES zenity
		PATHS ENV PATH
	)
	if(not ZENITY_FOUND)
		message(FATAL_ERROR "zenity not found. Please install zenity first.")
	endif()
endif()

find_package(Boost COMPONENTS filesystem process REQUIRED)

# checkers
set(checkers_exe_list)
add_subdirectory(checkers/)

# main
add_executable(oit-stress)
target_sources(oit-stress
	INTERFACE
		src/base/common.hpp
		src/base/config.hpp
		src/base/message.hpp
		src/base/process.hpp
	PRIVATE
		src/main.cpp
		src/core/cleaner.cpp
		src/core/info.cpp
		src/core/judger.cpp
		src/core/options.cpp
		src/core/tester.cpp
)
target_include_directories(oit-stress PRIVATE src)

target_link_libraries(oit-stress ${Boost_LIBRARIES})
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_link_libraries(oit-stress wsock32 ws2_32)
endif()

# installing
install(TARGETS oit-stress ${checkers_exe_list}
	RUNTIME DESTINATION bin/
)
