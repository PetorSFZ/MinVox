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

World::World(const std::string& name)
:
	mHorizontalChunkRange{NUM_CHUNK_RANGE},
	mVerticalChunkRange{NUM_CHUNK_RANGE},
	mCurrentOffsetY{0},
	mCurrentOffsetZ{0},
	mCurrentOffsetX{0},
	mChunks{numChunks(NUM_CHUNK_RANGE)},
	name{name}
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