

namespace vox {

// ChunkIndex & iterators
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline vec3f ChunkIndex::part8Offset() const noexcept
{
	return vec3f{static_cast<float>(part8X()*8),
	             static_cast<float>(part8Y()*8),
	             static_cast<float>(part8Z()*8)};
}

inline vec3f ChunkIndex::part4Offset() const noexcept
{
	return vec3f{static_cast<float>(part8X()*8 + part4X()*4),
	             static_cast<float>(part8Y()*8 + part4Y()*4),
	             static_cast<float>(part8Z()*8 + part4Z()*4)};
}

inline vec3f ChunkIndex::part2Offset() const noexcept
{
	return vec3f{static_cast<float>(part8X()*8 + part4X()*4 + part2X()*2),
	             static_cast<float>(part8Y()*8 + part4Y()*4 + part2Y()*2),
	             static_cast<float>(part8Z()*8 + part4Z()*4 + part2Z()*2)};
}

inline vec3f ChunkIndex::voxelOffset() const noexcept
{
	return vec3f{static_cast<float>(part8X()*8 + part4X()*4 + part2X()*2 + voxelX()),
	             static_cast<float>(part8Y()*8 + part4Y()*4 + part2Y()*2 + voxelY()),
	             static_cast<float>(part8Z()*8 + part4Z()*4 + part2Z()*2 + voxelZ())};
}

// ChunkPart2: Getters & setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline Voxel ChunkPart2::getVoxel(size_t x, size_t y, size_t z) const noexcept
{
	sfz_assert_debug(x < 2);
	sfz_assert_debug(y < 2);
	sfz_assert_debug(z < 2);
	return mVoxel[x][y][z];
}

inline Voxel ChunkPart2::getVoxel(const vec3i& offset) const noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	return getVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2]);
}

inline Voxel ChunkPart2::getVoxel(ChunkIndex index) const noexcept
{
	return mVoxel[index.voxelX()][index.voxelY()][index.voxelZ()];
}

inline void ChunkPart2::setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept
{
	sfz_assert_debug(x < 2);
	sfz_assert_debug(y < 2);
	sfz_assert_debug(z < 2);
	mVoxel[x][y][z] = voxel;
}

inline void ChunkPart2::setVoxel(const vec3i& offset, Voxel voxel) noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	setVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2], voxel);
}

inline void ChunkPart2::setVoxel(ChunkIndex index, Voxel voxel) noexcept
{
	mVoxel[index.voxelX()][index.voxelY()][index.voxelZ()] = voxel;
}

// Chunk: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline Chunk::Chunk() noexcept
{
	static_assert(sizeof(Voxel) == 1, "Voxel is padded.");
	static_assert(sizeof(ChunkPart2) == 8, "ChunkPart2 is padded.");
	static_assert(sizeof(ChunkPart4) == 64, "ChunkPart4 is padded.");
	static_assert(sizeof(ChunkPart8) == 512, "ChunkPart8 is padded.");
	static_assert(sizeof(Chunk) == 4096, "Chunk is padded.");
}

// Chunk: Getters & setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline Voxel Chunk::getVoxel(size_t x, size_t y, size_t z) const noexcept
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

inline Voxel Chunk::getVoxel(const vec3i& offset) const noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	return getVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2]);
}

inline Voxel Chunk::getVoxel(ChunkIndex i) const noexcept
{
	sfz_assert_debug(i.mIndex < ChunkIterateEnd.mIndex);
	const ChunkPart8* part8 = &mChunkPart8s[i.part8X()][i.part8Y()][i.part8Z()];
	const ChunkPart4* part4 = &part8->mChunkPart4s[i.part4X()][i.part4Y()][i.part4Z()];
	const ChunkPart2* part2 = &part4->mChunkPart2s[i.part2X()][i.part2Y()][i.part2Z()];
	return part2->getVoxel(i);
}

inline void Chunk::setVoxel(size_t x, size_t y, size_t z, Voxel voxel) noexcept
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

inline void Chunk::setVoxel(const vec3i& offset, Voxel voxel) noexcept
{
	sfz_assert_debug(0 <= offset[0]);
	sfz_assert_debug(0 <= offset[1]);
	sfz_assert_debug(0 <= offset[2]);
	setVoxel((size_t)offset[0], (size_t)offset[1], (size_t)offset[2], voxel);
}

// Chunk AABB calculators
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

inline void calculateChunkAABB(AABB& aabb, const vec3f& chunkPos) noexcept
{
	static const vec3f chunkSize{16.0f, 16.0f, 16.0f};
	aabb.min(chunkPos);
	aabb.max(chunkPos + chunkSize);
}

inline void calculateChunkPart8AABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept
{
	static const vec3f chunkPart8Size{8.0f, 8.0f, 8.0f};
	vec3f minPos = chunkPos + index.part8Offset();
	aabb.min(minPos);
	aabb.max(minPos + chunkPart8Size);
}

inline void calculateChunkPart4AABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept
{
	static const vec3f chunkPart4Size{4.0f, 4.0f, 4.0f};
	vec3f minPos = chunkPos + index.part4Offset();
	aabb.min(minPos);
	aabb.max(minPos + chunkPart4Size);
}

inline void calculateChunkPart2AABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept
{
	static const vec3f chunkPart2Size{2.0f, 2.0f, 2.0f};
	vec3f minPos = chunkPos + index.part2Offset();
	aabb.min(minPos);
	aabb.max(minPos + chunkPart2Size);
}

inline void calculateVoxelAABB(AABB& aabb, const vec3f& chunkPos, ChunkIndex index) noexcept
{
	static const vec3f voxelSize{1.0f, 1.0f, 1.0f};
	vec3f minPos = chunkPos + index.voxelOffset();
	aabb.min(minPos);
	aabb.max(minPos + voxelSize);
}

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

