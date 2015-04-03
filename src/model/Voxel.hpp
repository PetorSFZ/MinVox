#pragma once
#ifndef VOX_MODEL_VOXELTYPE_HPP
#define VOX_MODEL_VOXELTYPE_HPP

#include <cstdint> // uint8_t

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::uint8_t;

/** Type of a voxel, takes up 4 bits. */
enum class VoxelType : uint8_t {
	AIR = 0,
	LIGHT = 1,
	BLUE = 2,
	GREEN = 3,
	ORANGE = 4,
	VANILLA = 5,
	YELLOW = 6
};

struct Voxel final {
	uint8_t mData;

	inline Voxel() noexcept;
	inline Voxel(uint8_t rawVoxel) noexcept;
	inline Voxel(VoxelType type) noexcept;
	inline Voxel(VoxelType type, uint8_t data) noexcept;

	inline VoxelType type() const noexcept;
	inline uint8_t extraData() const noexcept;
	inline void setType(VoxelType type) noexcept;
	inline void setExtraData(uint8_t data) noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#include "model/Voxel.inl"
#endif