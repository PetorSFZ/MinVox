#include "model/World.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

size_t calculateNumChunks(int horizontalChunkRange, int verticalChunkRange)
{
	size_t horizontalSide = (static_cast<size_t>(horizontalChunkRange)*2)+1;
	size_t slab = static_cast<size_t>(horizontalSide*horizontalSide);
	size_t verticalSide = (static_cast<size_t>(verticalChunkRange)*2)+1;
	return slab * verticalSide;
}

} // namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

World::World(const std::string& name) noexcept
:
	mHorizontalChunkRange{1},
	mVerticalChunkRange{1},
	mNumElements{calculateNumChunks(mHorizontalChunkRange, mVerticalChunkRange)},
	mChunks{new Chunk[mNumElements]},
	mOffsets{new vec3i[mNumElements]},
	mName{name}
{
	mCurrentChunkOffset = vec3i{0,0,0};
	size_t count = 0;
	for(int y = -mVerticalChunkRange; y <= mVerticalChunkRange; y++) {
		for (int z = -mHorizontalChunkRange; z <= mHorizontalChunkRange; z++) {
			for (int x = -mHorizontalChunkRange; x <= mHorizontalChunkRange; x++) {

				sfz_assert_debug(count < mNumElements);
				mOffsets[count] = vec3i{x, y, z};
				mChunks[count] = generateChunk(mOffsets[count]);
				count++;
			}
		}
	}
}

// Public member functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void World::update(const vec3f& camPos) noexcept
{
	vec3i oldChunkOffset = mCurrentChunkOffset;
	mCurrentChunkOffset = chunkOffsetFromPosition(camPos);

	if (oldChunkOffset != mCurrentChunkOffset) {
		// TODO: Streaming should happen here.
	}
}

const Chunk* World::chunkPtr(const vec3i& offset) const noexcept
{
	for (size_t i = 0; i < mNumElements; i++) {
		if (mOffsets[i] == offset) return &mChunks[i];
	}
	sfz_assert_debug_m(false, "Invalid chunk offset.");
	return nullptr;
}

const vec3i World::chunkOffset(const Chunk* chunkPtr) const noexcept
{
	for (size_t i = 0; i < mNumElements; i++) {
		if ((&mChunks[i]) == chunkPtr) return mOffsets[i];
	}
	sfz_assert_debug_m(false, "Invalid chunk pointer.");
	return vec3i{0,0,0};
}

const Chunk* World::chunkPtr(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumElements);
	return &mChunks[index];
}

vec3f World::positionFromChunkOffset(const vec3i& offset) const noexcept
{
	vec3i voxelOffset = offset * static_cast<int>(CHUNK_SIZE);
	return vec3f{(float)voxelOffset[0], (float)voxelOffset[1], (float)voxelOffset[2]};
}

vec3i World::chunkOffsetFromPosition(const vec3f& position) const noexcept
{
	int x = static_cast<int>(position[0]);
	int y = static_cast<int>(position[1]);
	int z = static_cast<int>(position[2]);
	return vec3i{y, z, x};
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>