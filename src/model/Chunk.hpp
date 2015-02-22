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

	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Voxel mVoxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	bitset_t mEmptyXRowFlags[CHUNK_SIZE];
	bitset_t mFullXRowFlags[CHUNK_SIZE];
	bitset_t mFullZRowFlags[CHUNK_SIZE];

	// Constructor & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Chunk(); 

	// Getters / Setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Voxel getVoxel(const Offset& offset) const;
	const Voxel* getVoxelPtr(const Offset& offset) const;
	void setVoxel(const Offset& offset, Voxel voxel);

	Voxel getVoxel(size_t y, size_t z, size_t x) const;
	const Voxel* getVoxelPtr(size_t y, size_t z, size_t x) const;
	void setVoxel(size_t y, size_t z, size_t x, Voxel voxel);

	// Query functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	bool isEmptyXRow(size_t y, size_t z) const;
	bool isEmptyLayer(size_t y) const;
	bool isEmptyChunk() const;

	bool isFullLayer(size_t y) const;
	bool isFullChunk() const;

	// Helper functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void updateAllFlags();

	void setEmptyXRowFlag(size_t y, size_t z);
	void clearEmptyXRowFlag(size_t y, size_t z);
	bool checkEmptyXRowFlag(size_t y, size_t z) const;

	void setFullXRowFlag(size_t y, size_t z);
	void clearFullXRowFlag(size_t y, size_t z);
	bool checkFullXRowFlag(size_t y, size_t z) const;

	void setFullZRowFlag(size_t y, size_t x);
	void clearFullZRowFlag(size_t y, size_t x);
	bool checkFullZRowFlag(size_t y, size_t z) const;
};

} // namespace vox

#endif