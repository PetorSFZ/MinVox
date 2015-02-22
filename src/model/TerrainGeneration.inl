namespace vox {

inline Chunk generateChunk(const Offset& offset)
{
	Chunk chunk;

	for (size_t z = 0; z < CHUNK_SIZE; z++) {
		for (size_t x = 0; x < CHUNK_SIZE; x++) {
			chunk.setVoxel(0, z, x, Voxel{VoxelType::EARTH, 0});
		}
	}
	return chunk;
}

} // namespace vox