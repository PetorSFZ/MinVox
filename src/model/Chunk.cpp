#include "model/Chunk.hpp"

namespace vox {

Chunk::Chunk()
{
	static_assert(sizeof(Voxel) == 1, "Voxel should be 1 byte large");
}

Voxel Chunk::getVoxel(size_t y, size_t z, size_t x) const
{
	return mVoxels[y][z][x];
}

const Voxel* Chunk::getVoxelPtr(size_t y, size_t z, size_t x) const
{
	return &mVoxels[y][z][x];
}


} // namespace vox