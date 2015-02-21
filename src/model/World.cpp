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
	size_t count = 0;
	ChunkOffset offset;
	for(int y = -VERTICAL_CHUNK_RANGE; y <= VERTICAL_CHUNK_RANGE; y++) {
		for (int z = -HORIZONTAL_CHUNK_RANGE; z <= HORIZONTAL_CHUNK_RANGE; z++) {
			for (int x = -HORIZONTAL_CHUNK_RANGE; x <= HORIZONTAL_CHUNK_RANGE; x++) {
			
				offset.set(y, z, x);
				mChunks[count] = generateChunk(offset);
				count++;
				sfz_assert_debug(count <= numChunks(HORIZONTAL_CHUNK_RANGE, VERTICAL_CHUNK_RANGE));
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