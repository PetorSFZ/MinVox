#include "model/Chunk.hpp"

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Chunk::Chunk()
{
	static_assert(sizeof(Voxel) == 1, "Voxel should be 1 byte large");

	// Since default chunk is only air all rows are empty.
	for (bitset_t& flags : mEmptyRowFlags) {
		flags = std::numeric_limits<bitset_t>::max();
	}
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

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
			clearEmptyRowFlag(y, z);
			return;
		}
	}
	setEmptyRowFlag(y, z);
}

bool Chunk::isEmptyRow(size_t y, size_t z) const
{
	bitset_t layerBits = mEmptyRowFlags[y];
	layerBits >>= z;
	layerBits &= 1;
	return layerBits;
}

bool Chunk::isEmptyLayer(size_t y) const
{
	bitset_t layerBits = mEmptyRowFlags[y];
	return (layerBits == std::numeric_limits<bitset_t>::max());
}

// Helper functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void Chunk::setEmptyRowFlag(size_t y, size_t z)
{
	bitset_t flag = (1 << z);
	mEmptyRowFlags[y] |= flag;
}

void Chunk::clearEmptyRowFlag(size_t y, size_t z)
{
	bitset_t flag = ~(1 << z);
	mEmptyRowFlags[y] &= flag;
}

} // namespace vox