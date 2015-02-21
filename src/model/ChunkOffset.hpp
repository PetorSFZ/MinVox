#pragma once
#ifndef VOX_MODEL_CHUNK_OFFSET_HPP
#define VOX_MODEL_CHUNK_OFFSET_HPP

namespace vox {

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

} // namespace vox

#endif