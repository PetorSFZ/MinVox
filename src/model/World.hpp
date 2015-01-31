#pragma once
#ifndef VOX_MODEL_WORLD_HPP
#define VOX_MODEL_WORLD_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <string>
#include <memory>

#include <sfz/Math.hpp>

#include "model/Voxel.hpp"
#include "model/Chunk.hpp"
#include "io/ChunkIO.hpp"

namespace vox {

using std::size_t;
using std::unique_ptr;
using sfz::vec3f;

/** The numbers of chunk loaded in any given direction from the current chunk.
    I.e. a value of 2 would mean that 5x5x5 chunks are loaded at the same time. */
const size_t NUM_CHUNK_RANGE = 2;

/** Struct used to specify the offset of a chunk from the middle (0,0,0) of the World. */
struct ChunkOffset final {
	int mY, mZ, mX;

	inline ChunkOffset(int y, int z, int x)
	:
		mY{y},
		mZ{z},
		mX{x}
	{
		// Initialization done.
	}
};

class World final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	World(const std::string& name);

	// Public member functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update(const vec3f& basePos);
	const Chunk* chunkPtr(ChunkOffset offset) const;
	const ChunkOffset chunkOffset(const Chunk* chunkPtr) const;

private:
	// Private member functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	ChunkOffset offsetFromPosition(const vec3f& pos) const;

	// Private Members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	size_t mHorizontalChunkRange;
	size_t mVerticalChunkRange;
	ChunkOffset mCurrentOffset;
	unique_ptr<Chunk[]> mChunks;
	std::string mName;
};

} // namespace vox

#endif