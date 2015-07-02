

namespace vox {

inline Chunk generateChunk(const vec3i& offset) noexcept
{
	vec3i worldOffset = offset * static_cast<int>(CHUNK_SIZE);
	vec3i worldOffsetOrig = worldOffset;
	Chunk chunk;

	for (int y = 0; y < (int)CHUNK_SIZE; y++) {
		for (int z = 0; z < (int)CHUNK_SIZE; z++) {
			for (int x = 0; x < (int)CHUNK_SIZE; x++) {

				chunk.setVoxel((size_t)x,(size_t)y,(size_t)z, generateVoxel(worldOffset));
				
				worldOffset[0] += 1;
			}
			worldOffset[2] += 1;
			worldOffset[0] = worldOffsetOrig[0];
		}
		worldOffset[1] += 1;
		worldOffset[2] = worldOffsetOrig[2];
		worldOffset[0] = worldOffsetOrig[0];
	}

	//chunk.updateAllFlags();

	return chunk;
}

inline Voxel generateVoxel(const vec3i& worldOffset) noexcept
{
	auto heightFunc = [](int x, int z) -> int { return -0.05f*(x-10)*(x-20) - 0.05f*(z-10)*(z-20) + 4; };

	// Ground
	if (worldOffset[1] == 0) return Voxel{VOXEL_VANILLA};

	if (worldOffset[1] == heightFunc(worldOffset[0], worldOffset[2])) {
		return Voxel{VOXEL_BLUE};
	}
	
	return Voxel{VOXEL_AIR};
	sfz_assert_debug(false);
}

} // namespace vox

