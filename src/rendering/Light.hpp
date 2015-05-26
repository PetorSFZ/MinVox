#pragma once
#ifndef VOX_RENDERING_LIGHT_HPP
#define VOX_RENDERING_LIGHT_HPP

#include <sfz/Math.hpp>
#include "rendering/Camera.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec3f;
using sfz::mat4f;

// DirectionalLight
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct DirectionalLight final {
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Camera mCam;
	float mRange;
	vec3f mColor;

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	DirectionalLight(const vec3f& position, const vec3f& direction, const vec3f& upVector,
	                 float verticalFov, float near, float far) noexcept;

	DirectionalLight(const vec3f& pos, const vec3f& dir, float near, float range,
	                 const vec3f& color) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update() noexcept;
	mat4f lightMatrix(const mat4f& inverseViewMatrix) const noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif