#include "model/Chunk.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// ChunkPart2: Getters & setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Voxel ChunkPart2::getVoxel(size_t x, size_t y, size_t z) const noexcept
{
	sfz_assert_debug(x < 2);
	sfz_assert_debug(y < 2);
	sfz_assert_debug(z < 2);
	return mVoxel[x][y][z];
}

Voxel ChunkPart2::getVoxel(const vec3i& offset) const noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	return getVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2]);
}

void ChunkPart2::setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept
{
	sfz_assert_debug(x < 2);
	sfz_assert_debug(y < 2);
	sfz_assert_debug(z < 2);
	mVoxel[x][y][z] = voxel;
}

void ChunkPart2::setVoxel(const vec3i& offset, Voxel voxel) noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	setVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2], voxel);
}

// Chunk: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Chunk::Chunk() noexcept
{
	static_assert(sizeof(Voxel) == 1, "Voxel is padded.");
	static_assert(sizeof(ChunkPart2) == 8, "ChunkPart2 is padded.");
	static_assert(sizeof(ChunkPart4) == 64, "ChunkPart4 is padded.");
	static_assert(sizeof(ChunkPart8) == 512, "ChunkPart8 is padded.");
	static_assert(sizeof(Chunk) == 4096, "Chunk is padded.");
}

// Chunk: Getters & setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Voxel Chunk::getVoxel(size_t x, size_t y, size_t z) const noexcept
{
	sfz_assert_debug(x < CHUNK_SIZE);
	sfz_assert_debug(y < CHUNK_SIZE);
	sfz_assert_debug(z < CHUNK_SIZE);

	size_t xi, yi, zi;
	
	if (x < 8) xi = 0;
	else xi = 1;
	if (y < 8) yi = 0;
	else yi = 1;
	if (z < 8) zi = 0;
	else zi = 1;
	const ChunkPart8* part8 = &mChunkPart8s[xi][yi][zi];

	x %= 8;
	y %= 8;
	z %= 8;
	if (x < 4) xi = 0;
	else xi = 1;
	if (y < 4) yi = 0;
	else yi = 1;
	if (z < 4) zi = 0;
	else zi = 1;
	const ChunkPart4* part4 = &part8->mChunkPart4s[xi][yi][zi];

	x %= 4;
	y %= 4;
	z %= 4;
	if (x < 2) xi = 0;
	else xi = 1;
	if (y < 2) yi = 0;
	else yi = 1;
	if (z < 2) zi = 0;
	else zi = 1;
	const ChunkPart2* part2 = &part4->mChunkPart2s[xi][yi][zi];

	x %= 2;
	y %= 2;
	z %= 2;

	return part2->getVoxel(x, y, z);
}

Voxel Chunk::getVoxel(const vec3i& offset) const noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	return getVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2]);
}

void Chunk::setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept
{
	sfz_assert_debug(x < CHUNK_SIZE);
	sfz_assert_debug(y < CHUNK_SIZE);
	sfz_assert_debug(z < CHUNK_SIZE);

	size_t xi, yi, zi;
	
	if (x < 8) xi = 0;
	else xi = 1;
	if (y < 8) yi = 0;
	else yi = 1;
	if (z < 8) zi = 0;
	else zi = 1;
	ChunkPart8* part8 = &mChunkPart8s[xi][yi][zi];

	x %= 8;
	y %= 8;
	z %= 8;
	if (x < 4) xi = 0;
	else xi = 1;
	if (y < 4) yi = 0;
	else yi = 1;
	if (z < 4) zi = 0;
	else zi = 1;
	ChunkPart4* part4 = &part8->mChunkPart4s[xi][yi][zi];

	x %= 4;
	y %= 4;
	z %= 4;
	if (x < 2) xi = 0;
	else xi = 1;
	if (y < 2) yi = 0;
	else yi = 1;
	if (z < 2) zi = 0;
	else zi = 1;
	ChunkPart2* part2 = &part4->mChunkPart2s[xi][yi][zi];

	x %= 2;
	y %= 2;
	z %= 2;

	part2->setVoxel(x, y, z, voxel);
}

void Chunk::setVoxel(const vec3i& offset, Voxel voxel) noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	setVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2], voxel);
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>