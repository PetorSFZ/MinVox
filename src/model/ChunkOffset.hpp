#pragma once
#ifndef VOX_MODEL_CHUNK_OFFSET_HPP
#define VOX_MODEL_CHUNK_OFFSET_HPP

#include <string>

namespace vox {

/** Struct used to specify the offset of a chunk from the middle (0,0,0) of the World. */
struct ChunkOffset final {
	int mY, mZ, mX;

	inline ChunkOffset() = default;

	inline ChunkOffset(int y, int z, int x);

	inline void set(int y, int z, int x);
	inline std::string to_string() const;

	inline bool operator== (const ChunkOffset& other);
	inline bool operator!= (const ChunkOffset& other);
};

} // namespace vox

#include "model/ChunkOffset.inl"
#endif