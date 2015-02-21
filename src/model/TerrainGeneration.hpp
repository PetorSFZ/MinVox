#pragma once
#ifndef VOX_MODEL_TERRAIN_GENERATION_HPP
#define VOX_MODEL_TERRAIN_GENERATION_HPP

#include "model/Chunk.hpp"
#include "model/ChunkOffset.hpp"
#include "model/Voxel.hpp"

namespace vox {

inline Chunk generateChunk(const ChunkOffset& offset);

} // namespace vox

#include "model/TerrainGeneration.inl"
#endif