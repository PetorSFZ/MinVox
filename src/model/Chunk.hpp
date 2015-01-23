#pragma once
#ifndef VOX_MODEL_CHUNK_HPP
#define VOX_MODEL_CHUNK_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t

#include "model/Voxel.hpp"

namespace vox {

using std::uint8_t;
using std::size_t;

const size_t CHUNK_SIZE = 16;

class Chunk {
public:
	Chunk();

	Voxel getVoxel(size_t y, size_t z, size_t x) const;
	const Voxel* getVoxelPtr(size_t y, size_t z, size_t x) const;

private:
	Voxel mVoxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

} // namespace vox

#endif