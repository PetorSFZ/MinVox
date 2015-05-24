#pragma once
#ifndef VOX_MODEL_VOXELTYPE_HPP
#define VOX_MODEL_VOXELTYPE_HPP

#include <cstdint> // uint8_t

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::uint8_t;

const uint8_t VOXEL_AIR = 0;
const uint8_t VOXEL_LIGHT = 1;
const uint8_t VOXEL_BLUE = 2;
const uint8_t VOXEL_GREEN = 3;
const uint8_t VOXEL_ORANGE = 4;
const uint8_t VOXEL_VANILLA = 5;
const uint8_t VOXEL_YELLOW = 6;

struct Voxel final {
	uint8_t mType;

	inline Voxel(const Voxel& other) noexcept = default;
	inline Voxel& operator= (const Voxel& other) noexcept = default;

	inline Voxel() noexcept;
	inline Voxel(uint8_t type) noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#include "model/Voxel.inl"
#endif