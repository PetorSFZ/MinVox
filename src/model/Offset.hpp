#pragma once
#ifndef VOX_MODEL_OFFSET_HPP
#define VOX_MODEL_OFFSET_HPP

#include <string>

#include <sfz/math/Vector.hpp>

namespace vox {

/**
 * @brief Struct used to specify an offset in the model. 
 * It could be an offset to a Chunk from the middle (0,0,0) of the World, or it could be an offset
 * from the base of a Chunk to a specific Voxel inside it.
 */
struct Offset final {
	int mY, mZ, mX;

	inline Offset() = default;
	inline Offset(const Offset&) = default;
	inline Offset& operator= (const Offset&) = default;
	inline ~Offset() = default;

	inline Offset(int y, int z, int x);
	inline void set(int y, int z, int x);

	inline sfz::vec3f toVector() const;
	inline std::string to_string() const;

	inline bool operator== (const Offset& other);
	inline bool operator!= (const Offset& other);
};

/** Converts a ChunkOffset to a VoxelOffset. I.e. the offset will point the bottom-left-front 
    voxel of the pointed at Chunk. */
inline Offset chunkToVoxelOffset(const Offset& offset, int chunkSize);

} // namespace vox

#include "model/Offset.inl"
#endif