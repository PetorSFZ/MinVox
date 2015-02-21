#include "model/World.hpp"

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

size_t numChunks(size_t horizontalChunkRange, size_t verticalChunkRange)
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
	mHorizontalChunkRange{HORIZONTAL_CHUNK_RANGE},
	mVerticalChunkRange{VERTICAL_CHUNK_RANGE},
	mCurrentOffset{0 ,0, 0},
	mChunks{new Chunk[numChunks(HORIZONTAL_CHUNK_RANGE, VERTICAL_CHUNK_RANGE)]},
	mName{name}
{
	for (size_t i = 0; i < numChunks(HORIZONTAL_CHUNK_RANGE, VERTICAL_CHUNK_RANGE); i++) {
		mChunks[i] = Chunk{};
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
	// TODO: Proper implementation
	return &mChunks[0];
}

const ChunkOffset World::chunkOffset(const Chunk* chunkPtr) const
{
	// TODO: Proper implementation
	return ChunkOffset{0, 0, 0};
}

// Private member functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ChunkOffset World::offsetFromPosition(const vec3f& pos) const
{
	// TODO: Proper implementation.
	return ChunkOffset{0,0,0};
}

} // namespace vox