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
#include "model/Offset.hpp"
#include "io/ChunkIO.hpp"

namespace vox {

using std::size_t;
using std::unique_ptr;
using sfz::vec3f;

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

	void update(const vec3f& camPos);
	const Chunk* chunkPtr(const Offset& offset) const;
	const Offset chunkOffset(const Chunk* chunkPtr) const;

	const Chunk* chunkPtr(size_t index) const;
	inline size_t numChunks() const { return mNumElements; }

	vec3f positionFromChunkOffset(const Offset& offset) const;
	Offset chunkOffsetFromPosition(const vec3f& position) const;

	inline Offset currentChunkOffset() const { return mCurrentChunkOffset; }

private:
	// Private Members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	int mHorizontalChunkRange;
	int mVerticalChunkRange;
	size_t mNumElements;
	unique_ptr<Chunk[]> mChunks;
	unique_ptr<Offset[]> mOffsets;
	Offset mCurrentChunkOffset;
	
	std::string mName;
};

} // namespace vox

#endif