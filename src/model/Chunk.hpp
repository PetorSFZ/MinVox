#pragma once
#ifndef VOX_MODEL_CHUNK_HPP
#define VOX_MODEL_CHUNK_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <limits> //std::numeric_limits

#include "model/Voxel.hpp"
#include "model/Offset.hpp"

namespace vox {

using std::uint8_t;
using std::size_t;

const size_t CHUNK_SIZE = 16;

struct Chunk final {
	using bitset_t = std::uint16_t;

	Voxel mVoxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	bitset_t mEmptyRowFlags[CHUNK_SIZE];

	Chunk();

	Voxel getVoxel(const Offset& offset) const;
	const Voxel* getVoxelPtr(const Offset& offset) const;
	void setVoxel(const Offset& offset, Voxel voxel);

	Voxel getVoxel(size_t y, size_t z, size_t x) const;
	const Voxel* getVoxelPtr(size_t y, size_t z, size_t x) const;
	void setVoxel(size_t y, size_t z, size_t x, Voxel voxel);

	bool isEmptyRow(size_t y, size_t z) const;
	bool isEmptyLayer(size_t y) const;

	void setEmptyRowFlag(size_t y, size_t z);
	void clearEmptyRowFlag(size_t y, size_t z);
};

} // namespace vox

#endif