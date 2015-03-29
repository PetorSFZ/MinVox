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

World::World(const std::string& name, const vec3f& camPos,
             size_t horizontalRange, size_t verticalRange) noexcept
:
	mHorizontalRange{static_cast<int>(horizontalRange)},
	mVerticalRange{static_cast<int>(verticalRange)},
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

vec3i World::chunkOffsetFromPosition(const vec3i& position) const noexcept
{
	vec3i offset = position / (int)CHUNK_SIZE;
	vec3i voxelOffset = position - (offset*(int)CHUNK_SIZE);
	if (voxelOffset[0] < 0) offset[0]--;
	if (voxelOffset[1] < 0) offset[1]--;
	if (voxelOffset[2] < 0) offset[2]--;
	return offset;
}

vec3i World::chunkOffsetFromPosition(const vec3f& position) const noexcept
{
	return chunkOffsetFromPosition(vec3i{(int)position[0], (int)position[1], (int)position[2]});
}

void World::setVoxel(const vec3i& position, Voxel voxel) noexcept
{
	vec3i chunkOffset = chunkOffsetFromPosition(position);
	vec3i voxelOffset = position - (chunkOffset*(int)CHUNK_SIZE);

	int index = chunkIndex(chunkOffset);
	if (index == -1) return;
	Chunk* chunkPtr = &mChunks[index];
	Voxel oldVoxel = chunkPtr->getVoxel(voxelOffset);
	
	chunkPtr->setVoxel(voxelOffset, voxel);
	bool success = writeChunk(*chunkPtr, chunkOffset[0], chunkOffset[1], chunkOffset[2], mName);
	if (!success) {
		chunkPtr->setVoxel(voxelOffset, oldVoxel);
	}
}

void World::setVoxel(const vec3f& position, Voxel voxel) noexcept
{
	setVoxel(vec3i{(int)position[0], (int)position[1], (int)position[2]}, voxel);
}

// Getters / setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

size_t World::chunkIndex(const Chunk* chunkPtr) const noexcept
{
	return chunkPtr - (&mChunks[0]);
}

int World::chunkIndex(const vec3i& offset) const noexcept
{
	for (int i = 0; i < (int)mNumChunks; i++) {
		if (mOffsets[i] == offset && mAvailabilities[i]) return i;
	}
	return -1;
}


const Chunk* World::chunkPtr(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumChunks);
	return &mChunks[index];
}

const vec3i World::chunkOffset(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumChunks);
	return mOffsets[index];
}

bool World::chunkAvailable(size_t index) const noexcept
{
	sfz_assert_debug(index < mNumChunks);
	return mAvailabilities[index];
}

Voxel World::getVoxel(const vec3i& offset) const noexcept
{
	vec3i chunkOffset = chunkOffsetFromPosition(offset);
	vec3i voxelOffset = offset - (chunkOffset*(int)CHUNK_SIZE);

	int index = chunkIndex(chunkOffset);
	if (index == -1) return Voxel{VoxelType::AIR, 0};
	Chunk* chunkPtr = &mChunks[index];
	return chunkPtr->getVoxel(voxelOffset);
}

Voxel World::getVoxel(const vec3f& position) const noexcept
{
	return getVoxel(vec3i{(int)position[0], (int)position[1], (int)position[2]});
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