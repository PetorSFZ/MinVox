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
#include "io/ChunkIO.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::size_t;
using std::unique_ptr;
using sfz::vec3f;
using sfz::vec3i;

class World final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	World() = delete;
	World(const std::string& name) noexcept;

	// Public member functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update(const vec3f& camPos) noexcept;
	const Chunk* chunkPtr(const vec3i& offset) const noexcept;
	const vec3i chunkOffset(const Chunk* chunkPtr) const noexcept;

	const Chunk* chunkPtr(size_t index) const noexcept;
	inline size_t numChunks() const noexcept { return mNumElements; }

	vec3f positionFromChunkOffset(const vec3i& offset) const noexcept;
	vec3i chunkOffsetFromPosition(const vec3f& position) const noexcept;

	inline vec3i currentChunkOffset() const noexcept { return mCurrentChunkOffset; }

private:
	// Private Members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	int mHorizontalChunkRange;
	int mVerticalChunkRange;
	size_t mNumElements;
	unique_ptr<Chunk[]> mChunks;
	unique_ptr<vec3i[]> mOffsets;
	vec3i mCurrentChunkOffset;
	
	std::string mName;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif