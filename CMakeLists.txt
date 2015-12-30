cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project("MinVox")

# Sets build type to release if no build type is specified in a single-configuration generator.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

# Compiler flags
if(MSVC)
	# MSVC flags
	set(CMAKE_CXX_FLAGS "/Wall /Zi /EHsc /wd4514 /wd4668 /wd4710 /wd4711 /wd4820")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /DEBUG /DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG  "/Od /DEBUG")
else()
	# GCC / Clang flags
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-c++11-extensions -std=c++11 -pthread")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -ffast-math -g -DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DSFZ_NO_DEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
endif()

# Directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${SRC_DIR}) # Same as source dir since not a library
set(EXTERNALS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# SkipIfZero Common
add_subdirectory(${EXTERNALS_DIR}/SkipIfZeroCommon)

# Include directories
include_directories(
	${INCLUDE_DIR}
	${SFZ_COMMON_INCLUDE_DIRS}
)

# Source files
set(SOURCE_BASE_FILES
	${SRC_DIR}/GlobalConfig.hpp
	${SRC_DIR}/GlobalConfig.cpp
	${SRC_DIR}/IO.hpp
	${SRC_DIR}/Model.hpp
	${SRC_DIR}/Rendering.hpp
	${SRC_DIR}/Screens.hpp)
source_group(vox FILES ${SOURCE_BASE_FILES})

set(SOURCE_IO_FILES
	${SRC_DIR}/io/ChunkIO.hpp
	${SRC_DIR}/io/ChunkIO.cpp
	${SRC_DIR}/io/IOUtils.hpp
	${SRC_DIR}/io/IOUtils.cpp)
source_group(vox_io FILES ${SOURCE_IO_FILES})

set(SOURCE_MODEL_FILES
	${SRC_DIR}/model/Chunk.hpp
	${SRC_DIR}/model/Chunk.inl
	${SRC_DIR}/model/ChunkMesh.hpp
	${SRC_DIR}/model/ChunkMesh.cpp
	${SRC_DIR}/model/TerrainGeneration.hpp
	${SRC_DIR}/model/TerrainGeneration.inl
	${SRC_DIR}/model/Voxel.hpp
	${SRC_DIR}/model/Voxel.inl
	${SRC_DIR}/model/World.hpp
	${SRC_DIR}/model/World.cpp)
source_group(vox_model FILES ${SOURCE_MODEL_FILES})

set(SOURCE_RENDERING_FILES
	${SRC_DIR}/rendering/Assets.hpp
	${SRC_DIR}/rendering/Assets.cpp
	${SRC_DIR}/rendering/CubeObject.hpp
	${SRC_DIR}/rendering/CubeObject.cpp
	${SRC_DIR}/rendering/Shaders.hpp
	${SRC_DIR}/rendering/Shaders.cpp
	${SRC_DIR}/rendering/SkyCubeObject.hpp
	${SRC_DIR}/rendering/SkyCubeObject.cpp
	${SRC_DIR}/rendering/SSAO.hpp
	${SRC_DIR}/rendering/SSAO.cpp
	${SRC_DIR}/rendering/WorldRenderer.hpp
	${SRC_DIR}/rendering/WorldRenderer.cpp)
source_group(vox_rendering FILES ${SOURCE_RENDERING_FILES})

set(SOURCE_SCREENS_FILES
	${SRC_DIR}/screens/GameScreen.hpp
	${SRC_DIR}/screens/GameScreen.cpp)
source_group(vox_screens FILES ${SOURCE_SCREENS_FILES})

set(SOURCE_ALL_FILES
	${SOURCE_BASE_FILES}
	${SOURCE_IO_FILES}
	${SOURCE_MODEL_FILES}
	${SOURCE_RENDERING_FILES}
	${SOURCE_SCREENS_FILES})

# Main executable
add_executable(MinVox ${SRC_DIR}/Main.cpp ${SOURCE_ALL_FILES})

# Linking libraries to main executable
target_link_libraries(
	MinVox

	${SFZ_COMMON_LIBRARIES}
)

# Xcode specific file copying
if(CMAKE_GENERATOR STREQUAL Xcode)
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/Debug)
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/Release)
endif()

# MSVC specific file copying
if(MSVC)
	
	# Assets folder
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/Debug)
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/Release)

	# Runtime DLLs
	file(COPY RuntimeDLLs/msvc12_x64_debug/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR})

	file(COPY RuntimeDLLs/msvc12_x64_release/SDL2.dll DESTINATION ${CMAKE_BINARY_DIR})

endif()
