cmake_minimum_required (VERSION 3.15)

project (simple_json_plugin VERSION 1.5.3 DESCRIPTION "Firebird streaming Simple JSON plugin" LANGUAGES CXX)

# set variable for the C preprocessor to detect the operation system
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(LINUX)
	set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build/simple_json_plugin/linux-x64)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(_WINDOWS)
	if (CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64")
	    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build/simple_json_plugin/windows-x64)
	endif()
	if (CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "X86")
	    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build/simple_json_plugin/windows-x86)
	endif()
endif()


###############################################################################
# Require and enable C++ 0x/11/14/17
############
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "${PROJECT_NAME} C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "${PROJECT_NAME} C++ compiler: ${CMAKE_CXX_COMPILER_ID}")

add_compile_definitions(HAVE_CONFIG_H)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
    add_compile_definitions(LINUX)
  	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
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
	message(STATUS "${PROJECT_NAME} Compiler flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "${PROJECT_NAME} Compiler flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

message(STATUS "${PROJECT_NAME} Binary directory: ${CMAKE_BINARY_DIR}")

####################################
# src
####################################
file(GLOB_RECURSE PROJECT_SOURCES
    "../../src/include/*.h"
    "../../src/common/*"
	"../../src/encoding/*"
    "../../src/plugins/simple_json/*")

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${FIREBIRD_INCLUDE_DIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(nlohmann_json CONFIG REQUIRED)
endif()


if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME} PRIVATE -lstdc++fs)
    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
endif()

set(STREAMING_DIR /opt/fb_streaming)
set(PLUGINS_DIR /opt/fb_streaming/stream_plugins)

install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGINS_DIR}/)

# Install documentation
install(
	FILES "../../doc/simple_json_plugin.md" "../../doc/simple_json_plugin_ru.md"
	DESTINATION ${STREAMING_DIR}/doc
	PERMISSIONS
		OWNER_READ
		GROUP_READ
		WORLD_READ
)
