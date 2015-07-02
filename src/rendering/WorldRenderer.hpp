#pragma once
#ifndef VOX_RENDERING_WORLD_RENDERER_HPP
#define VOX_RENDERING_WORLD_RENDERER_HPP

#include <sfz/Math.hpp>
#include "sfz/GL.hpp"

#include "rendering/Assets.hpp"
#include "rendering/Camera.hpp"
#include "rendering/CubeObject.hpp"
#include "Model.hpp"



namespace vox {

using sfz::vec3;
using sfz::mat4;

class WorldRenderer {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	WorldRenderer() = delete;
	WorldRenderer(const WorldRenderer&) = delete;
	WorldRenderer(WorldRenderer&&) = delete;
	WorldRenderer& operator= (const WorldRenderer&) = delete;
	WorldRenderer& operator= (WorldRenderer&&) = delete;

	WorldRenderer(const World& world) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void drawWorld(const Camera& cam, int modelMatrixLoc) noexcept;
	void drawWorldOld(const Camera& cam, int modelMatrixLoc) noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	const World& mWorld;
	CubeObject mCubeObj;
};

} // namespace vox


#endif