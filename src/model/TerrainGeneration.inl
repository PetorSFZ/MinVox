namespace vox {

inline Chunk generateChunk(const Offset& offset)
{
	Offset worldOffset = chunkToVoxelOffset(offset, (int)CHUNK_SIZE);
	Chunk chunk;

	for (int y = 0; y < (int)CHUNK_SIZE; y++) {
		for (int z = 0; z < (int)CHUNK_SIZE; z++) {
			for (int x = 0; x < (int)CHUNK_SIZE; x++) {

				chunk.setVoxel((size_t)y,(size_t)z,(size_t)x, generateVoxel(worldOffset));
				
				worldOffset.mX += 1;
			}
			worldOffset.mZ += 1;
		}
		worldOffset.mY += 1;
	}

	return chunk;
}

inline Voxel generateVoxel(const Offset& worldOffset)
{
	if (worldOffset.mY == 0) return Voxel{VoxelType::GRASS, 0};
	else if (worldOffset.mY < 0) return Voxel{VoxelType::EARTH, 0};
	return Voxel{VoxelType::AIR, 0};
}

} // namespace vox