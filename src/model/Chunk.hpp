#pragma once
#ifndef VOX_MODEL_CHUNK_HPP
#define VOX_MODEL_CHUNK_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <limits> //std::numeric_limits

#include <sfz/Math.hpp>

#include "model/Voxel.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::uint8_t;
using std::size_t;
using sfz::vec3i;

const size_t CHUNK_SIZE = 16;

class ChunkPart2 {
private:
	Voxel mVoxel[2][2][2];
public:
	Voxel getVoxel(size_t x, size_t y, size_t z) const noexcept;
	Voxel getVoxel(const vec3i& offset) const noexcept;
	void setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept;
	void setVoxel(const vec3i& offset, Voxel voxel) noexcept;
};

struct ChunkPart4 {
	ChunkPart2 mChunkPart2s[2][2][2];
};

struct ChunkPart8 {
	ChunkPart4 mChunkPart4s[2][2][2];
};

struct Chunk {
	ChunkPart8 mChunkPart8s[2][2][2];

	Chunk() noexcept;

	Voxel getVoxel(size_t x, size_t y, size_t z) const noexcept;
	Voxel getVoxel(const vec3i& offset) const noexcept;
	void setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept;
	void setVoxel(const vec3i& offset, Voxel voxel) noexcept;
};
/*
inline Offset chunkToVoxelOffset(const Offset& offset, int chunkSize)
{
	return Offset{offset.mY*chunkSize, offset.mZ*chunkSize, offset.mX*chunkSize};
}*/

/*struct Chunk final {
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
};*/

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif