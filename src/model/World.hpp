#pragma once
#ifndef VOX_MODEL_WORLD_HPP
#define VOX_MODEL_WORLD_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <string>
#include <memory>

#include <sfz/Assert.hpp>
#include <sfz/Math.hpp>

#include "model/Voxel.hpp"
#include "model/Chunk.hpp"
#include "model/ChunkOffset.hpp"
#include "io/ChunkIO.hpp"

namespace vox {

using std::size_t;
using std::unique_ptr;
using sfz::vec3f;

/** The numbers of chunks loaded in horizontal direction from the current chunk. */
const size_t HORIZONTAL_CHUNK_RANGE = 2;

/** The number of chunks loaded in vertical direction from the current chunk. */
const size_t VERTICAL_CHUNK_RANGE = 2;

class World final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	World() = delete;
	World(const World&) = delete;
	World& operator= (const World&) = delete;

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