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
	mHorizontalRange{2},
	mVerticalRange{1},
	mNumChunks{calculateNumChunks(mHorizontalRange, mVerticalRange)},
	mChunks{new Chunk[mNumChunks]},
	mOffsets{new vec3i[mNumChunks]},
	mAvailabilities{new bool[mNumChunks]},
	mName{name}
{
	mCurrentChunkOffset = vec3i{0,0,0};
	size_t count = 0;
	for(int y = -mVerticalRange; y <= mVerticalRange; y++) {
		for (int z = -mHorizontalRange; z <= mHorizontalRange; z++) {
			for (int x = -mHorizontalRange; x <= mHorizontalRange; x++) {

				sfz_assert_debug(count < mNumChunks);
				mOffsets[count] = vec3i{x, y, z};
				if (!readChunk(mChunks[count], x, y, z, mName)) {
					mChunks[count] = generateChunk(mOffsets[count]);
				}
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
		vec3i minOffset = mCurrentChunkOffset - vec3i{mHorizontalRange, mVerticalRange, mHorizontalRange};
		vec3i maxOffset = mCurrentChunkOffset + vec3i{mHorizontalRange, mVerticalRange, mHorizontalRange};

		size_t count = 0; 
		for (int x = minOffset[0]; x <= maxOffset[0]; x++) {
			for (int y = minOffset[1]; y <= maxOffset[1]; y++) {
				for (int z = minOffset[2]; z <= maxOffset[2]; z++) {
					mOffsets[count] = vec3i{x, y, z};
					if (!readChunk(mChunks[count], x, y, z, mName)) {
						mChunks[count] = generateChunk(mOffsets[count]);
					}
					mAvailabilities[count] = true;
					count++;
				}
			}
		}
	}
}

vec3f World::positionFromChunkOffset(const vec3i& offset) const noexcept
{
	vec3i voxelOffset = offset * static_cast<int>(CHUNK_SIZE);
	return vec3f{(float)voxelOffset[0], (float)voxelOffset[1], (float)voxelOffset[2]};
}

vec3i World::chunkOffsetFromPosition(const vec3f& position) const noexcept
{
	int x = static_cast<int>(std::round(position[0]/(float)CHUNK_SIZE));
	int y = static_cast<int>(std::round(position[1])/(float)CHUNK_SIZE);
	int z = static_cast<int>(std::round(position[2])/(float)CHUNK_SIZE);
	return vec3i{x, y, z};
}

// Getters / setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const Chunk* World::chunkPtr(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumChunks);
	return &mChunks[index];
}

const Chunk* World::chunkPtr(const vec3i& offset) const noexcept
{
	for (size_t i = 0; i < mNumChunks; i++) {
		if (mOffsets[i] == offset) return &mChunks[i];
	}
	sfz_assert_debug_m(false, "Invalid chunk offset.");
	return nullptr;
}


const vec3i World::chunkOffset(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumChunks);
	return mOffsets[index];
}

const vec3i World::chunkOffset(const Chunk* chunkPtr) const noexcept
{
	sfz_assert_debug(&mChunks[0] <= chunkPtr && chunkPtr <= &mChunks[mNumChunks-1]);
	return mOffsets[chunkPtr - (&mChunks[0])];
}


bool World::chunkAvailable(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumChunks);
	return mAvailabilities[index];
}

bool World::chunkAvailable(const Chunk* chunkPtr) const noexcept
{
	sfz_assert_debug(&mChunks[0] <= chunkPtr && chunkPtr <= &mChunks[mNumChunks-1]);
	return mAvailabilities[chunkPtr - (&mChunks[0])];
}

bool World::chunkAvailable(const vec3i& offset) const noexcept
{
	for (size_t i = 0; i < mNumChunks; i++) {
		if (mOffsets[i] == offset) return mAvailabilities[i];
	}
	return false;
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>