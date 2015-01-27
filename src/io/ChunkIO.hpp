#pragma once
#ifndef VOX_IO_CHUNK_IO_HPP
#define VOX_IO_CHUNK_IO_HPP

#include <string>
#include <iostream>
#include <cstdio>

#include "VoxModel.hpp"
#include "io/IOUtils.hpp"

namespace vox {

using std::size_t;

bool readChunk(Chunk& chunk, int yOffset, int zOffset, int xOffset, const std::string& worldName);
bool writeChunk(Chunk& chunk, int yOffset, int zOffset, int xOffset, const std::string& worldName);

} // namespace vox

#endif