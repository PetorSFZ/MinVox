namespace vox {

inline Chunk generateChunk(const Offset& offset)
{
	Offset worldOffset = chunkToVoxelOffset(offset, (int)CHUNK_SIZE);
	Offset worldOffsetOrig = worldOffset;
	Chunk chunk;

	for (int y = 0; y < (int)CHUNK_SIZE; y++) {
		for (int z = 0; z < (int)CHUNK_SIZE; z++) {
			for (int x = 0; x < (int)CHUNK_SIZE; x++) {

				chunk.setVoxel((size_t)y,(size_t)z,(size_t)x, generateVoxel(worldOffset));
				
				worldOffset.mX += 1;
			}
			worldOffset.mZ += 1;
			worldOffset.mX = worldOffsetOrig.mX;
		}
		worldOffset.mY += 1;
		worldOffset.mZ = worldOffsetOrig.mZ;
		worldOffset.mX = worldOffsetOrig.mX;
	}

	//chunk.updateAllFlags();

	return chunk;
}

inline Voxel generateVoxel(const Offset& worldOffset)
{
	auto heightFunc = [](int x, int z) -> int { return -0.05f*(x-10)*(x-20) - 0.05f*(z-10)*(z-20) + 4; };

	// Ground
	if (worldOffset.mY == 0) return Voxel{VoxelType::VANILLA, 0};
	if (worldOffset.mY == -1) return Voxel{VoxelType::VANILLA, 0};

	if (worldOffset.mY == heightFunc(worldOffset.mX, worldOffset.mZ)) {
		return Voxel{VoxelType::BLUE, 0};
	}
	
	return Voxel{VoxelType::AIR, 0};
	sfz_assert_debug(false);
}

} // namespace vox