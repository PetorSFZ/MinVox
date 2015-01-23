cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project("MinVox")

# Compiler flags
if(MSVC)
	# MSVC flags
	#set(cpp_compiler_flags "/Wall /EHsc /O2")
	set(cpp_compiler_flags "/O2")
else() # Assume we're compiling with CLANG or GCC if not MSVC.
	# GCC and Clang flags
	set(cpp_compiler_flags "-Wall -Wextra -Wno-c++11-extensions -std=c++11 -O3 -pthread")
endif()

# Sets the specified compiler flags, will only set the first time cmake configures project.
if(NOT MINVOX_CONFIGURED_ONCE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cpp_compiler_flags}" 
	    CACHE STRING "Flags used by the compiler during all build types." FORCE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${c_compiler_flags}"
	    CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

# Directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${SRC_DIR}) # Same as source dir since not a library
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)
set(CMAKE_MODULES ${CMAKE_CURRENT_LIST_DIR}/cmake)

# Adds our cmake modules to module path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_MODULES})

# Finds SDL2 and SDL2_image, will make use of $SDL2 environment variable on Windows.
find_package(SDL2 REQUIRED) # ${SDL2_INCLUDE_DIR}, ${SDL2_LIBRARY}
find_package(SDL2_image REQUIRED) # ${SDL2_IMAGE_INCLUDE_DIR}, ${SDL2_IMAGE_LIBRARY}

# GLEW
# find_package(GLEW REQUIRED) # ${GLEW_INCLUDE_DIRS}, ${GLEW_LIBRARIES}
add_definitions(-DGLEW_STATIC)
add_subdirectory(${EXTERNALS_DIR}/glew)

# Links OpenGL
find_package(OpenGL REQUIRED) # ${OPENGL_INCLUDE_DIR}, ${OPENGL_LIBRARIES}

# Include directories
include_directories(
	${INCLUDE_DIR}
	${SDL2_INCLUDE_DIR}
	${SDL2_IMAGE_INCLUDE_DIR}
	${GLEW_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
)

# Main executable
add_executable(
	MinVox

	${SRC_DIR}/Main.cpp
)

# Linking libraries to main executable
target_link_libraries(
	MinVox

	${SDL2_LIBRARY}
	${SDL2_IMAGE_LIBRARY}
	${GLEW_LIBRARIES}
	${OPENGL_LIBRARIES}
)

# Xcode specific file copying
if(CMAKE_GENERATOR STREQUAL Xcode)
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/Debug)
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/Release)
endif()

# MSVC specific file copying
if(MSVC)
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Sets flag that ensures some variables are only set once.
set(MINVOX_CONFIGURED_ONCE TRUE CACHE INTERNAL "A flag showing that CMake has configured at least once.")
