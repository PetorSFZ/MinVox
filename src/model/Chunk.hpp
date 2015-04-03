#pragma once
#ifndef VOX_MODEL_CHUNK_HPP
#define VOX_MODEL_CHUNK_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <limits> //std::numeric_limits

#include <sfz/Math.hpp>
#include <sfz/Geometry.hpp>

#include "model/Voxel.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::uint8_t;
using std::uint16_t;
using std::size_t;
using sfz::vec3f;
using sfz::vec3i;
using sfz::AABB;

const size_t CHUNK_SIZE = 16;

// ChunkIndex & iterators
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct ChunkIndex final {
	uint16_t mIndex;

	ChunkIndex() noexcept = default;
	inline ChunkIndex(uint16_t index) : mIndex{index} {}

	static inline uint16_t valueOfBit(uint16_t value, uint16_t position) noexcept
	{
		return (value >> position) & uint16_t(1);
	}

	inline uint16_t part8X() const noexcept { return valueOfBit(mIndex, 11); }
	inline uint16_t part8Y() const noexcept { return valueOfBit(mIndex, 10); }
	inline uint16_t part8Z() const noexcept { return valueOfBit(mIndex, 9); }

	inline uint16_t part4X() const noexcept { return valueOfBit(mIndex, 8); }
	inline uint16_t part4Y() const noexcept { return valueOfBit(mIndex, 7); }
	inline uint16_t part4Z() const noexcept { return valueOfBit(mIndex, 6); }

	inline uint16_t part2X() const noexcept { return valueOfBit(mIndex, 5); }
	inline uint16_t part2Y() const noexcept { return valueOfBit(mIndex, 4); }
	inline uint16_t part2Z() const noexcept { return valueOfBit(mIndex, 3); }

	inline uint16_t voxelX() const noexcept { return valueOfBit(mIndex, 2); }
	inline uint16_t voxelY() const noexcept { return valueOfBit(mIndex, 1); }
	inline uint16_t voxelZ() const noexcept { return valueOfBit(mIndex, 0); }

	inline vec3f part8Offset() const noexcept;
	inline vec3f part4Offset() const noexcept;
	inline vec3f part2Offset() const noexcept;
	inline vec3f voxelOffset() const noexcept;

	inline void plusPart8() noexcept { mIndex += (uint16_t(1 << 9)); }
	inline void plusPart4() noexcept { mIndex += (uint16_t(1 << 6)); }
	inline void plusPart2() noexcept { mIndex += (uint16_t(1 << 3)); }
	inline void plusVoxel() noexcept { mIndex++; };

	inline void operator++ (int) noexcept { mIndex++; }
	inline ChunkIndex& operator-- () noexcept { mIndex--; return *this; }
	inline bool operator== (const ChunkIndex& o) const noexcept { return mIndex == o.mIndex; }
	inline bool operator!= (const ChunkIndex& o) const noexcept { return mIndex != o.mIndex; }
};

// Can iterate through all the voxels in a Chunk using the following loop:
// for (ChunkIndex i = ChunkIterateBegin; i < ChunkIterateEnd; i++) { }
const ChunkIndex ChunkIterateBegin{0};
const ChunkIndex ChunkIterateEnd{uint16_t(1) << 12};

// ChunkParts
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

#define COMPACT_CHUNK_PART_2

#ifdef COMPACT_CHUNK_PART_2
class ChunkPart2 {
private:
	uint8_t mVoxelData[2][2];
public:
	inline Voxel getVoxel(size_t x, size_t y, size_t z) const noexcept;
	inline Voxel getVoxel(const vec3i& offset) const noexcept;
	inline Voxel getVoxel(ChunkIndex index) const noexcept;
	inline void setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept;
	inline void setVoxel(const vec3i& offset, Voxel voxel) noexcept;
	inline void setVoxel(ChunkIndex index, Voxel voxel) noexcept;
};
#else
class ChunkPart2 {
private:
	Voxel mVoxel[2][2][2];
public:
	inline Voxel getVoxel(size_t x, size_t y, size_t z) const noexcept;
	inline Voxel getVoxel(const vec3i& offset) const noexcept;
	inline Voxel getVoxel(ChunkIndex index) const noexcept;
	inline void setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept;
	inline void setVoxel(const vec3i& offset, Voxel voxel) noexcept;
	inline void setVoxel(ChunkIndex index, Voxel voxel) noexcept;
};
#endif

struct ChunkPart4 {
	ChunkPart2 mChunkPart2s[2][2][2];
	inline const ChunkPart2* chunkPart2Ptr(const vec3i& offset) const noexcept
	{
		return &mChunkPart2s[offset[0]][offset[1]][offset[2]];
	}
};

struct ChunkPart8 {
	ChunkPart4 mChunkPart4s[2][2][2];
	inline const ChunkPart4* chunkPart4Ptr(const vec3i& offset) const noexcept
	{
		return &mChunkPart4s[offset[0]][offset[1]][offset[2]];
	}
};

// Chunk
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct Chunk {
	ChunkPart8 mChunkPart8s[2][2][2];
	inline const ChunkPart8* chunkPart8Ptr(const vec3i& offset) const noexcept
	{
		return &mChunkPart8s[offset[0]][offset[1]][offset[2]];
	}

	inline Chunk() noexcept;

	inline Voxel getVoxel(size_t x, size_t y, size_t z) const noexcept;
	inline Voxel getVoxel(const vec3i& offset) const noexcept;
	inline Voxel getVoxel(ChunkIndex index) const noexcept;
	inline void setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept;
	inline void setVoxel(const vec3i& offset, Voxel voxel) noexcept;
};

// Chunk AABB calculators
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline void calculateChunkAABB(AABB& aabb, const vec3f& chunkPos) noexcept;
inline void calculateChunkPart8AABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept;
inline void calculateChunkPart4AABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept;
inline void calculateChunkPart2AABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept;
inline void calculateVoxelAABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept;

inline void calculateChunkAABB(AABB& aabb, const vec3i& chunkOffset) noexcept;
inline void calculateChunkPart8AABB(AABB& aabb, const vec3i& chunkOffset,
                                                const vec3i& part8Offset) noexcept;
inline void calculateChunkPart4AABB(AABB& aabb, const vec3i& chunkOffset,
                                                const vec3i& part8Offset,
                                                const vec3i& part4Offset) noexcept;
inline void calculateChunkPart2AABB(AABB& aabb, const vec3i& chunkOffset,
                                                const vec3i& part8Offset,
                                                const vec3i& part4Offset,
                                                const vec3i& part2Offset) noexcept;
inline void calculateVoxelAABB(AABB& aabb, const vec3i& chunkOffset,
                                           const vec3i& part8Offset,
                                           const vec3i& part4Offset,
                                           const vec3i& part2Offset,
                                           const vec3i& voxelOffset) noexcept;

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#include "model/Chunk.inl"
#endif