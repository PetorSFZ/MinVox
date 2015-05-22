#pragma once
#ifndef VOX_MODEL_CHUNK_MESH_HPP
#define VOX_MODEL_CHUNK_MESH_HPP

#include <sfz/math/Vector.hpp>
#include "model/Chunk.hpp"
#include <memory>

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec2f;
using sfz::vec3f;
using std::unique_ptr;

// ChunkMesh
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class ChunkMesh final {
public:

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	ChunkMesh(const ChunkMesh&) = delete;
	ChunkMesh& operator= (const ChunkMesh&) = delete;

	ChunkMesh() noexcept;
	~ChunkMesh() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void set(const Chunk& chunk) noexcept;
	void render() const noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	unsigned int mVAO;
	unsigned int mVertexBuffer, mNormalBuffer, mUVBuffer, mIndexBuffer;

	size_t mCurrentNumVoxels = 0;
	const size_t mNumVoxelsPerChunk, mDataArraySize, mIndicesArraySize;
	const unique_ptr<vec3f[]> mVertexArray;
	const unique_ptr<vec3f[]> mNormalArray;
	const unique_ptr<vec2f[]> mUVArray;
	const unique_ptr<unsigned int[]> mIndexArray;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>

#endif