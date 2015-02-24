#include "model/World.hpp"

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

World::World(const std::string& name)
:
	mHorizontalChunkRange{0},
	mVerticalChunkRange{1},
	mNumElements{calculateNumChunks(mHorizontalChunkRange, mVerticalChunkRange)},
	mChunks{new Chunk[mNumElements]},
	mOffsets{new Offset[mNumElements]},
	mCurrentChunkOffset{0 ,0, 0},
	mName{name}
{
	size_t count = 0;
	for(int y = -mVerticalChunkRange; y <= mVerticalChunkRange; y++) {
		for (int z = -mHorizontalChunkRange; z <= mHorizontalChunkRange; z++) {
			for (int x = -mHorizontalChunkRange; x <= mHorizontalChunkRange; x++) {

				sfz_assert_debug(count < mNumElements);
				mOffsets[count] = Offset{y, z, x};
				mChunks[count] = generateChunk(mOffsets[count]);
				count++;
			}
		}
	}
}

// Public member functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void World::update(const vec3f& camPos)
{
	Offset oldChunkOffset = mCurrentChunkOffset;
	mCurrentChunkOffset = chunkOffsetFromPosition(camPos);

	if (oldChunkOffset != mCurrentChunkOffset) {
		// TODO: Streaming should happen here.
	}
}

const Chunk* World::chunkPtr(const Offset& offset) const
{
	for (size_t i = 0; i < mNumElements; i++) {
		if (mOffsets[i] == offset) return &mChunks[i];
	}
	sfz_assert_debug_m(false, "Invalid chunk offset.");
	return nullptr;
}

const Offset World::chunkOffset(const Chunk* chunkPtr) const
{
	for (size_t i = 0; i < mNumElements; i++) {
		if ((&mChunks[i]) == chunkPtr) return mOffsets[i];
	}
	sfz_assert_debug_m(false, "Invalid chunk pointer.");
	return Offset{0,0,0};
}

const Chunk* World::chunkPtr(size_t index)
{
	sfz_assert_debug(index < mNumElements);
	return &mChunks[index];
}

vec3f World::positionFromChunkOffset(const Offset& offset) const
{
	return chunkToVoxelOffset(offset, static_cast<int>(CHUNK_SIZE)).toVector();
}

Offset World::chunkOffsetFromPosition(const vec3f& position) const
{
	int x = static_cast<int>(position[0]);
	int y = static_cast<int>(position[1]);
	int z = static_cast<int>(position[2]);
	return Offset{y, z, x};
}

} // namespace vox