#include "model/Chunk.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Getters & setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Voxel Chunk::getVoxel(size_t y, size_t z, size_t x) const noexcept
{
	sfz_assert_debug(0 <= y && y < CHUNK_SIZE);
	sfz_assert_debug(0 <= z && z < CHUNK_SIZE);
	sfz_assert_debug(0 <= x && x < CHUNK_SIZE);

	size_t yi, zi, xi;
	
	if (y < 8) yi = 0;
	else yi = 1;
	if (z < 8) zi = 0;
	else zi = 1;
	if (x < 8) xi = 0;
	else xi = 1;
	const ChunkPart8* part8 = &mChunkPart8s[yi][zi][xi];

	y %= 8;
	z %= 8;
	x %= 8;
	if (y < 4) yi = 0;
	else yi = 1;
	if (z < 4) zi = 0;
	else zi = 1;
	if (x < 4) xi = 0;
	else xi = 1;
	const ChunkPart4* part4 = &part8->mChunkPart4s[yi][zi][xi];

	y %= 4;
	z %= 4;
	x %= 4;
	if (y < 2) yi = 0;
	else yi = 1;
	if (z < 2) zi = 0;
	else zi = 1;
	if (x < 2) xi = 0;
	else xi = 1;
	const ChunkPart2* part2 = &part4->mChunkPart2s[yi][zi][xi];

	y %= 2;
	z %= 2;
	x %= 2;

	return part2->mVoxel[y][z][x];
}

Voxel Chunk::getVoxel(const Offset& offset) const noexcept
{
	return getVoxel((size_t)offset.mY, (size_t)offset.mZ, (size_t)offset.mX);
}

void Chunk::setVoxel(size_t y, size_t z, size_t x, Voxel voxel) noexcept
{
	sfz_assert_debug(0 <= y && y < CHUNK_SIZE);
	sfz_assert_debug(0 <= z && z < CHUNK_SIZE);
	sfz_assert_debug(0 <= x && x < CHUNK_SIZE);

	size_t yi, zi, xi;
	
	if (y < 8) yi = 0;
	else yi = 1;
	if (z < 8) zi = 0;
	else zi = 1;
	if (x < 8) xi = 0;
	else xi = 1;
	ChunkPart8* part8 = &mChunkPart8s[yi][zi][xi];

	y %= 8;
	z %= 8;
	x %= 8;
	if (y < 4) yi = 0;
	else yi = 1;
	if (z < 4) zi = 0;
	else zi = 1;
	if (x < 4) xi = 0;
	else xi = 1;
	ChunkPart4* part4 = &part8->mChunkPart4s[yi][zi][xi];

	y %= 4;
	z %= 4;
	x %= 4;
	if (y < 2) yi = 0;
	else yi = 1;
	if (z < 2) zi = 0;
	else zi = 1;
	if (x < 2) xi = 0;
	else xi = 1;
	ChunkPart2* part2 = &part4->mChunkPart2s[yi][zi][xi];

	y %= 2;
	z %= 2;
	x %= 2;

	part2->mVoxel[y][z][x] = voxel;
}

void Chunk::setVoxel(const Offset& offset, Voxel voxel) noexcept
{
	setVoxel((size_t)offset.mY, (size_t)offset.mZ, (size_t)offset.mX, voxel);
}

/*
// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Chunk::Chunk()
{
	static_assert(sizeof(Voxel) == 1, "Voxel should be 1 byte large");

	// Since default chunk is only air all rows are empty.
	for (bitset_t& flags : mEmptyXRowFlags) {
		flags = std::numeric_limits<bitset_t>::max();
	}
}

// Getters / setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Voxel Chunk::getVoxel(const Offset& offset) const
{
	sfz_assert_debug(offset.mY >= 0);
	sfz_assert_debug(offset.mZ >= 0);
	sfz_assert_debug(offset.mX >= 0);
	return getVoxel((size_t)offset.mY, (size_t)offset.mZ, (size_t)offset.mX);
}

const Voxel* Chunk::getVoxelPtr(const Offset& offset) const
{
	sfz_assert_debug(offset.mY >= 0);
	sfz_assert_debug(offset.mZ >= 0);
	sfz_assert_debug(offset.mX >= 0);
	return getVoxelPtr((size_t)offset.mY, (size_t)offset.mZ, (size_t)offset.mX);
}

void Chunk::setVoxel(const Offset& offset, Voxel voxel)
{
	sfz_assert_debug(offset.mY >= 0);
	sfz_assert_debug(offset.mZ >= 0);
	sfz_assert_debug(offset.mX >= 0);
	updateAllFlags();
}

Voxel Chunk::getVoxel(size_t y, size_t z, size_t x) const
{
	return mVoxels[y][z][x];
}

const Voxel* Chunk::getVoxelPtr(size_t y, size_t z, size_t x) const
{
	return &mVoxels[y][z][x];
}

void Chunk::setVoxel(size_t y, size_t z, size_t x, Voxel voxel)
{
	mVoxels[y][z][x] = voxel;
	for (Voxel v : mVoxels[y][z]) {
		if (v.type() != VoxelType::AIR) {
			clearEmptyXRowFlag(y, z);
			return;
		}
	}
	setEmptyXRowFlag(y, z);
}

// Query functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

bool Chunk::isEmptyXRow(size_t y, size_t z) const
{
	return checkEmptyXRowFlag(y, z);
}

bool Chunk::isEmptyLayer(size_t y) const
{
	bitset_t layerBits = mEmptyXRowFlags[y];
	return (layerBits == std::numeric_limits<bitset_t>::max());
}

bool Chunk::isEmptyChunk() const
{
	for (size_t y = 0; y < CHUNK_SIZE; y++) {
		if (!isEmptyLayer(y)) return false;
	}
	return true;
}


bool Chunk::isFullLayer(size_t y) const
{
	bitset_t layerBits = mFullXRowFlags[y];
	return (layerBits == std::numeric_limits<bitset_t>::max());
}

bool Chunk::isFullChunk() const
{
	for (size_t y = 0; y < CHUNK_SIZE; y++) {
		if (!isFullLayer(y)) return false;
	}
	return true;
}

// Helper functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void Chunk::updateAllFlags()
{
	// Sets empty x row flags
	for (size_t y = 0; y < CHUNK_SIZE; y++) {
		for (size_t z = 0; z < CHUNK_SIZE; z++) {
			setEmptyXRowFlag(y, z);
			for (Voxel v : mVoxels[y][z]) {
				if (v.type() != VoxelType::AIR) {
					clearEmptyXRowFlag(y, z);
					break;
				}
			}	
		}
	}

	// Sets full x row flags
	for (size_t y = 0; y < CHUNK_SIZE; y++) {
		for (size_t z = 0; z < CHUNK_SIZE; z++) {
			setFullXRowFlag(y, z);
			for (Voxel v : mVoxels[y][z]) {
				if (v.type() == VoxelType::AIR) {
					clearFullXRowFlag(y, z);
					break;
				}
			}	
		}
	}

	// Sets full z row flags
	for (size_t y = 0; y < CHUNK_SIZE; y++) {
		for (size_t x = 0; x < CHUNK_SIZE; x++) {
			setFullZRowFlag(y, x);
			for (Voxel v : mVoxels[y][x]) {
				if (v.type() == VoxelType::AIR) {
					clearFullZRowFlag(y, x);
					break;
				}
			}	
		}
	}
}


void Chunk::setEmptyXRowFlag(size_t y, size_t z)
{
	bitset_t flag = static_cast<bitset_t>(1 << z);
	mEmptyXRowFlags[y] |= flag;
}

void Chunk::clearEmptyXRowFlag(size_t y, size_t z)
{
	bitset_t flag = (~static_cast<bitset_t>(1 << z));
	mEmptyXRowFlags[y] &= flag;
}

bool Chunk::checkEmptyXRowFlag(size_t y, size_t z) const
{
	bitset_t layerBits = mEmptyXRowFlags[y];
	layerBits >>= z;
	layerBits &= 1;
	return layerBits != 0;
}


void Chunk::setFullXRowFlag(size_t y, size_t z)
{
	bitset_t flag = static_cast<bitset_t>(1 << z);
	mFullXRowFlags[y] |= flag;
}

void Chunk::clearFullXRowFlag(size_t y, size_t z)
{
	bitset_t flag = (~static_cast<bitset_t>(1 << z));
	mFullXRowFlags[y] &= flag;
}

bool Chunk::checkFullXRowFlag(size_t y, size_t z) const
{
	bitset_t layerBits = mFullXRowFlags[y];
	layerBits >>= z;
	layerBits &= 1;
	return layerBits != 0;
}


void Chunk::setFullZRowFlag(size_t y, size_t x)
{
	bitset_t flag = static_cast<bitset_t>(1 << x);
	mFullZRowFlags[y] |= flag;
}

void Chunk::clearFullZRowFlag(size_t y, size_t x)
{
	bitset_t flag = (~static_cast<bitset_t>(1 << x));
	mFullZRowFlags[y] &= flag;
}

bool Chunk::checkFullZRowFlag(size_t y, size_t x) const
{
	bitset_t layerBits = mFullZRowFlags[y];
	layerBits >>= x;
	layerBits &= 1;
	return layerBits != 0;
}*/

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>