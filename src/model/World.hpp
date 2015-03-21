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
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	const int mHorizontalRange;
	const int mVerticalRange;
	const size_t mNumChunks;
	const std::string mName;

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	World(const std::string& name) noexcept;

	// Public member functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update(const vec3f& camPos) noexcept;

	vec3f positionFromChunkOffset(const vec3i& offset) const noexcept;

	vec3i chunkOffsetFromPosition(const vec3f& position) const noexcept;

	// Getters / setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	
	inline vec3i currentChunkOffset() const noexcept { return mCurrentChunkOffset; }

	const Chunk* chunkPtr(size_t index) const noexcept;
	const Chunk* chunkPtr(const vec3i& offset) const noexcept;

	const vec3i chunkOffset(size_t index) const noexcept;
	const vec3i chunkOffset(const Chunk* chunkPtr) const noexcept;

	bool chunkAvailable(size_t index) const noexcept;
	bool chunkAvailable(const Chunk* chunkPtr) const noexcept;
	bool chunkAvailable(const vec3i& offset) const noexcept;

private:
	// Private Members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	vec3i mCurrentChunkOffset;
	unique_ptr<Chunk[]> mChunks;
	unique_ptr<vec3i[]> mOffsets;
	unique_ptr<bool[]> mAvailabilities;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif