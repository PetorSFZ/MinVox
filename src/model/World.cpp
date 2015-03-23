#include "model/World.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

size_t calculateNumChunks(int horizontalChunkRange, int verticalChunkRange) noexcept
{
	size_t horizontalSide = (static_cast<size_t>(horizontalChunkRange)*2)+1;
	size_t slab = static_cast<size_t>(horizontalSide*horizontalSide);
	size_t verticalSide = (static_cast<size_t>(verticalChunkRange)*2)+1;
	return slab * verticalSide;
}

vec3i minChunkOffset(const World& world) noexcept
{
	vec3i range{world.mHorizontalRange, world.mVerticalRange, world.mHorizontalRange};
	return world.currentChunkOffset() - range;
}

vec3i maxChunkOffset(const World& world) noexcept
{
	vec3i range{world.mHorizontalRange, world.mVerticalRange, world.mHorizontalRange};
	return world.currentChunkOffset() + range;
}

bool outside(const vec3i& offset, const vec3i& min, const vec3i& max) noexcept
{
	return (offset[0] < min[0] || max[0] < offset[0]) ||
	       (offset[1] < min[1] || max[1] < offset[1]) ||
	       (offset[2] < min[2] || max[2] < offset[2]);
}

} // namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

World::World(const std::string& name, const vec3f& camPos) noexcept
:
	mHorizontalRange{2},
	mVerticalRange{1},
	mNumChunks{calculateNumChunks(mHorizontalRange, mVerticalRange)},
	mName{name},
	mChunks{new Chunk[mNumChunks]},
	mOffsets{new vec3i[mNumChunks]},
	mAvailabilities{new bool[mNumChunks]},
	mToBeReplaced{new bool[mNumChunks]}
{
	mCurrentChunkOffset = chunkOffsetFromPosition(camPos);

	for (size_t i = 0; i < mNumChunks; i++) {
		mOffsets[i] = vec3i{-100000000, -1000000000, -10000000};
		mAvailabilities[i] = false;
		mToBeReplaced[i] = true;
	}

	loadChunks();
}

// Public member functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void World::update(const vec3f& camPos) noexcept
{
	vec3i oldChunkOffset = mCurrentChunkOffset;
	mCurrentChunkOffset = chunkOffsetFromPosition(camPos);

	if (oldChunkOffset != mCurrentChunkOffset) {
		checkWhichChunksToReplace();
		loadChunks();
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

// Private methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void World::checkWhichChunksToReplace() noexcept
{
	const vec3i min = minChunkOffset(*this);
	const vec3i max = maxChunkOffset(*this);
	for (size_t i = 0; i < mNumChunks; i++) {
		mToBeReplaced[i] = outside(mOffsets[i], min, max);
	}
}

void World::loadChunks() noexcept
{
	const vec3i min = minChunkOffset(*this);
	const vec3i max = maxChunkOffset(*this);
	const vec3i end = offsetIterateEnd(min, max);
	vec3i itr = min;
	size_t currentWriteIndex = 0;
	size_t chunksLoaded = 0;

	while (itr != end) {
		bool offsetIsLoaded = false;
		for (size_t i = 0; i < mNumChunks; i++) {
			if (mOffsets[i] == itr && !mToBeReplaced[i]) {
				offsetIsLoaded = true;
				break;
			}
		}

		if (!offsetIsLoaded) {
			while (!mToBeReplaced[currentWriteIndex]) currentWriteIndex++;
			sfz_assert_debug(currentWriteIndex < mNumChunks);

			if (!readChunk(mChunks[currentWriteIndex], itr[0], itr[1], itr[2], mName)) {
				std::cout << "Generated and wrote chunk at: " << mOffsets[currentWriteIndex] << std::endl;
				mChunks[currentWriteIndex] = generateChunk(itr);
				writeChunk(mChunks[currentWriteIndex], itr[0], itr[1], itr[2], mName);
			}
			mOffsets[currentWriteIndex] = itr;
			mAvailabilities[currentWriteIndex] = true;
			mToBeReplaced[currentWriteIndex] = false;
			chunksLoaded++;
			currentWriteIndex++;
		}

		itr = offsetIterateNext(itr, min, max);
	}

	std::cout << "Loaded " << chunksLoaded << " chunks.\n";
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>