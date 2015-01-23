#pragma once
#ifndef VOX_VOXELTYPE_HPP
#define VOX_VOXELTYPE_HPP

#include <cstdint> // uint8_t

namespace vox {

using std::uint8_t;

/** Type of a voxel, takes up 3 bits. */
enum class VoxelType : uint8_t {
	AIR = 0,
	LIGHT = 1,
	MIRROR = 2,
	GRASS = 3,
	STONE = 4,
	METAL = 5,
	SAND = 6,
	WATER = 7
};

struct Voxel final {
	uint8_t mData;

	inline Voxel();
	inline Voxel(VoxelType type, uint8_t data);

	inline VoxelType type() const;
	inline uint8_t extraData() const;
	inline void setType(VoxelType type);
	inline void setExtraData(uint8_t data);
};

} // namespace vox

#include "model/Voxel.inl"
#endif