namespace vox {

inline Chunk generateChunk(const Offset& offset)
{
	Offset worldOffset = chunkToVoxelOffset(offset, (int)CHUNK_SIZE);
	Chunk chunk;

	for (int y = 0; y < (int)CHUNK_SIZE; y++) {
		for (int z = 0; z < (int)CHUNK_SIZE; z++) {
			for (int x = 0; x < (int)CHUNK_SIZE; x++) {
				
				if (worldOffset.mY == 0) chunk.setVoxel(y,z,x, Voxel{VoxelType::GRASS, 0});
				else if (worldOffset.mY < 0) chunk.setVoxel(y,z,x, Voxel{VoxelType::EARTH, 0});

				worldOffset.mX += 1;
			}
			worldOffset.mZ += 1;
		}
		worldOffset.mY += 1;
	}

	return chunk;
}

} // namespace vox