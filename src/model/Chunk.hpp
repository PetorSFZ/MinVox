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
using std::size_t;
using sfz::vec3f;
using sfz::vec3i;
using sfz::AABB;

const size_t CHUNK_SIZE = 16;

// ChunkParts
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

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

	Chunk() noexcept;

	Voxel getVoxel(size_t x, size_t y, size_t z) const noexcept;
	Voxel getVoxel(const vec3i& offset) const noexcept;
	void setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept;
	void setVoxel(const vec3i& offset, Voxel voxel) noexcept;
};

// ChunkPart iterator functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline vec3i chunkPartIterateBegin() noexcept { return vec3i{0,0,0}; }
inline vec3i chunkPartIterateEnd() noexcept { return vec3i{2,0,0}; }
inline vec3i chunkPartIterateNext(const vec3i& current) noexcept
{
	vec3i next = current;
	next[2]++;
	if (next[2] >= 2) {
		next[2] = 0;
		next[1]++;
		if (next[1] >= 2) {
			next[1] = 0;
			next[0]++;
		}
	}
	return next;
}

inline vec3i offsetIterateNext(const vec3i& current, const vec3i& min, const vec3i& max) noexcept
{
	vec3i next = current;
	next[2]++;
	if (next[2] > max[2]) {
		next[2] = min[2];
		next[1]++;
		if (next[1] > max[1]) {
			next[1] = min[1];
			next[0]++;
		}
	}
	return next;
}

inline vec3i offsetIterateEnd(const vec3i& min, const vec3i& max) noexcept
{
	return vec3i{max[0] + 1, min[1], min[2]};
}

// Chunk AABB calculators
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline void calculateChunkAABB(AABB& aabb, const vec3i& chunkOffset) noexcept
{
	static vec3f chunkSize{16.0f, 16.0f, 16.0f};
	vec3i minTemp = chunkOffset*16;
	vec3f minPos{(float)minTemp[0], (float)minTemp[1], (float)minTemp[2]};
	aabb.min(minPos);
	aabb.max(minPos + chunkSize);
}

inline void calculateChunkPart8AABB(AABB& aabb, const vec3i& chunkOffset,
                                                const vec3i& part8Offset) noexcept
{
	static vec3f chunkPart8Size{8.0f, 8.0f, 8.0f};
	vec3i minTemp = chunkOffset*16 + part8Offset*8;
	vec3f minPos{(float)minTemp[0], (float)minTemp[1], (float)minTemp[2]};
	aabb.min(minPos);
	aabb.max(minPos + chunkPart8Size);
}

inline void calculateChunkPart4AABB(AABB& aabb, const vec3i& chunkOffset,
                                                const vec3i& part8Offset,
                                                const vec3i& part4Offset) noexcept
{
	static vec3f chunkPart4Size{4.0f, 4.0f, 4.0f};
	vec3i minTemp = chunkOffset*16 + part8Offset*8 + part4Offset*4;
	vec3f minPos{(float)minTemp[0], (float)minTemp[1], (float)minTemp[2]};
	aabb.min(minPos);
	aabb.max(minPos + chunkPart4Size);
}

inline void calculateChunkPart2AABB(AABB& aabb, const vec3i& chunkOffset,
                                                const vec3i& part8Offset,
                                                const vec3i& part4Offset,
                                                const vec3i& part2Offset) noexcept
{
	static vec3f chunkPart2Size{2.0f, 2.0f, 2.0f};
	vec3i minTemp = chunkOffset*16 + part8Offset*8 + part4Offset*4 + part2Offset*2;
	vec3f minPos{(float)minTemp[0], (float)minTemp[1], (float)minTemp[2]};
	aabb.min(minPos);
	aabb.max(minPos + chunkPart2Size);
}

inline void calculateVoxelAABB(AABB& aabb, const vec3i& chunkOffset,
                                           const vec3i& part8Offset,
                                           const vec3i& part4Offset,
                                           const vec3i& part2Offset,
                                           const vec3i& voxelOffset) noexcept
{
	static vec3f voxelSize{1.0f, 1.0f, 1.0f};
	vec3i minTemp = chunkOffset*16 + part8Offset*8 + part4Offset*4 + part2Offset*2 + voxelOffset;
	vec3f minPos{(float)minTemp[0], (float)minTemp[1], (float)minTemp[2]};
	aabb.min(minPos);
	aabb.max(minPos + voxelSize);
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif