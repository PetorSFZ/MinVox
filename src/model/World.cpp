#include "model/World.hpp"

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

size_t numChunks(size_t numChunkRange)
{
	size_t numSide = (numChunkRange*2) + 1;
	return numSide*numSide*numSide;
}

} // namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

World::World()
:
	mCurrentChunkRange{NUM_CHUNK_RANGE},
	mBaseOffsetY{0},
	mBaseOffsetZ{0},
	mBaseOffsetX{0},
	mChunks{numChunks(NUM_CHUNK_RANGE)}
{
	for (size_t i = 0; i < numChunks(NUM_CHUNK_RANGE); i++) {
		mChunks.emplace_back();
	}
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const Chunk* World::chunkPtr(int offsetY, int offsetZ, int offsetX) const
{
	// TODO: Proper implementation
	return &mChunks[0];
}


} // namespace vox