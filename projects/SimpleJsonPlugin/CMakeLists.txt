cmake_minimum_required (VERSION 3.11)

project (simple_json_plugin VERSION 1.0.0)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/../../build)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

###############################################################################
# Require and enable C++ 0x/11/14/17
############
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "${PROJECT_NAME} compile: C++${CMAKE_CXX_STANDARD}")

add_compile_definitions(HAVE_CONFIG_H)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
    add_compile_definitions(LINUX)
  	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wnarrowing -Werror")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
	include(CheckCXXCompilerFlag)
	check_cxx_compiler_flag("-stdlib=libc++" CXX_SUPPORTS_STDLIB)
	if(CXX_SUPPORTS_STDLIB)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
		set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++")
	endif()	
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	string(REGEX REPLACE "[/-]W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	if (NOT (CMAKE_VERSION VERSION_LESS 3.6.0)) # Compiler features for Intel in CMake 3.6+
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd=c++17")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /QaxCORE-AVX2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise")
	set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Qipo")
elseif(MSVC)
	string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	if(MSVC_VERSION LESS 1900)
		message(FATAL_ERROR "${PROJECT_NAME} build: Build requires C++17-compliant compiler")
	endif()
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "${PROJECT_NAME} build: compiler flags - ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "${PROJECT_NAME} build: compiler flags - ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

####################################
# src
####################################
file(GLOB_RECURSE PROJECT_SOURCES 
    "../../src/include/*"
    "../../src/common/*"
    "../../src/plugins/simple_json/*")

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

find_package(ICU COMPONENTS uc REQUIRED)

include_directories(${ICU_INCLUDE_DIRS})

link_directories(${ICU_LIBRARY_DIRS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} -lstdc++fs)
target_link_libraries(${PROJECT_NAME} ICU::uc)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

set(PLUGINS_DIR /opt/fb_streaming/plugins)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGINS_DIR}/)
