#include "model/World.hpp"

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

size_t calculateNumChunks(size_t horizontalChunkRange, size_t verticalChunkRange)
{
	size_t horizontalSide = (horizontalChunkRange*2)+1;
	size_t slab = horizontalSide*horizontalSide;
	size_t verticalSide = (verticalChunkRange*2)+1;
	return slab * verticalSide;
}

} // namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

World::World(const std::string& name)
:
	mHorizontalChunkRange{2},
	mVerticalChunkRange{2},
	mNumElements{calculateNumChunks(mHorizontalChunkRange, mVerticalChunkRange)},
	mChunks{new Chunk[mNumElements]},
	mOffsets{new ChunkOffset[mNumElements]},
	mCurrentOffset{0 ,0, 0},
	mName{name}
{
	size_t count = 0;
	for(int y = -mVerticalChunkRange; y <= mVerticalChunkRange; y++) {
		for (int z = -mHorizontalChunkRange; z <= mHorizontalChunkRange; z++) {
			for (int x = -mHorizontalChunkRange; x <= mHorizontalChunkRange; x++) {

				sfz_assert_debug(count < mNumElements);
				mOffsets[count] = ChunkOffset{y, z, x};
				mChunks[count] = generateChunk(mOffsets[count]);
				count++;
			}
		}
	}
}

// Public member functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void World::update(const vec3f& basePos)
{
	// TODO: Check if new chunks needs to be streamed in and maybe do so.
}

const Chunk* World::chunkPtr(ChunkOffset offset) const
{
	for (size_t i = 0; i < mNumElements; i++) {
		if (mOffsets[i] == offset) return &mChunks[i];
	}
	sfz_assert_debug(false, "Invalid chunk offset.");
	return nullptr;
}

const ChunkOffset World::chunkOffset(const Chunk* chunkPtr) const
{
	for (size_t i = 0; i < mNumElements; i++) {
		if ((&mChunks[i]) == chunkPtr) return mOffsets[i];
	}
	sfz_assert_debug(false, "Invalid chunk pointer.");
	return ChunkOffset{0,0,0};
}

const Chunk* World::chunkPtr(size_t index)
{
	sfz_assert_debug(index < mNumElements);
	return &mChunks[index];
}

// Private member functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ChunkOffset World::offsetFromPosition(const vec3f& pos) const
{
	// TODO: Proper implementation.
	return ChunkOffset{0,0,0};
}

} // namespace vox