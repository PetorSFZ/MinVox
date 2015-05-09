#pragma once
#ifndef VOX_MODEL_CHUNK_MODEL_HPP
#define VOX_MODEL_CHUNK_MODEL_HPP

#include "sfz/GL.hpp"
#include "model/Chunk.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// ChunkModel
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class ChunkModel final {
public:

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	//ChunkModel() = delete;
	ChunkModel(const ChunkModel&) = delete;
	ChunkModel& operator= (const ChunkModel&) = delete;

	ChunkModel() noexcept;
	~ChunkModel() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void set(const Chunk& chunk) noexcept;
	void render() noexcept;

private:
	GLuint mVAO;
	GLuint mVertexBuffer, mNormalBuffer, mUVBuffer, mIndexBuffer, mPositionBuffer, mTexIDBuffer;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>

#endif