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
#include "model/ChunkMesh.hpp"
#include "io/ChunkIO.hpp"



namespace vox {

using std::size_t;
using std::unique_ptr;
using sfz::vec3;
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

	World(const std::string& name, const vec3& camPos,
	      size_t horizontalRange, size_t verticalRange) noexcept;

	// Public member functions
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update(const vec3& camPos) noexcept;

	vec3 positionFromChunkOffset(const vec3i& offset) const noexcept;

	vec3i chunkOffsetFromPosition(const vec3i& position) const noexcept;
	vec3i chunkOffsetFromPosition(const vec3& position) const noexcept;

	void setVoxel(const vec3i& position, Voxel voxel) noexcept;
	void setVoxel(const vec3& position, Voxel voxel) noexcept;

	// Getters / setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	
	inline vec3i currentChunkOffset() const noexcept { return mCurrentChunkOffset; }

	size_t chunkIndex(const Chunk* chunkPtr) const noexcept;
	int chunkIndex(const vec3i& offset) const noexcept;

	const Chunk* chunkPtr(size_t index) const noexcept;
	const ChunkMesh& chunkMesh(size_t index) const noexcept;
	const vec3i chunkOffset(size_t index) const noexcept;
	bool chunkAvailable(size_t index) const noexcept;

	Voxel getVoxel(const vec3i& offset) const noexcept;
	Voxel getVoxel(const vec3& position) const noexcept;

private:
	// Private methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void checkWhichChunksToReplace() noexcept;
	void loadChunks() noexcept;

	// Private Members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	vec3i mCurrentChunkOffset;
	unique_ptr<Chunk[]> mChunks;
	unique_ptr<ChunkMesh[]> mChunkMeshes;
	unique_ptr<vec3i[]> mOffsets;
	unique_ptr<bool[]> mAvailabilities;
	unique_ptr<bool[]> mToBeReplaced;
};

} // namespace vox


#endif