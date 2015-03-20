#pragma once
#ifndef VOX_MODEL_TERRAIN_GENERATION_HPP
#define VOX_MODEL_TERRAIN_GENERATION_HPP

#include <sfz/Math.hpp>

#include "model/Chunk.hpp"
#include "model/Voxel.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec3i;

inline Chunk generateChunk(const vec3i& offset) noexcept;

inline Voxel generateVoxel(const vec3i& worldOffset) noexcept;

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#include "model/TerrainGeneration.inl"
#endif