#pragma once
#ifndef VOX_MODEL_WORLD_HPP
#define VOX_MODEL_WORLD_HPP

#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <vector>
#include <string>

#include <sfz/Math.hpp>

#include "model/Voxel.hpp"
#include "model/Chunk.hpp"
#include "io/ChunkIO.hpp"

namespace vox {

using std::size_t;
using std::vector;

/** The numbers of chunk loaded in any given direction from the current chunk.
    I.e. a value of 2 would mean that 5x5x5 chunks are loaded at the same time. */
const size_t NUM_CHUNK_RANGE = 2;

class World final {
public:
	World(const std::string& name);

	const Chunk* chunkPtr(int offsetY, int offsetZ, int offsetX) const;

private:
	size_t mHorizontalChunkRange;
	size_t mVerticalChunkRange;
	int mCurrentOffsetY, mCurrentOffsetZ, mCurrentOffsetX;
	vector<Chunk> mChunks;
	std::string name;
};

} // namespace vox

#endif