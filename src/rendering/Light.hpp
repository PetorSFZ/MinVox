#pragma once
#ifndef VOX_RENDERING_LIGHT_HPP
#define VOX_RENDERING_LIGHT_HPP

#include <sfz/Math.hpp>
#include "rendering/Camera.hpp"
#include "sfz/GL.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec3f;
using sfz::vec4f;
using sfz::mat4f;

// DirectionalLight
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class DirectionalLightMesh final {
public:
	DirectionalLightMesh(const DirectionalLightMesh&) = delete;
	DirectionalLightMesh& operator= (const DirectionalLightMesh&) = delete;

	DirectionalLightMesh() noexcept;
	DirectionalLightMesh(float fov, float near, float far) noexcept;
	DirectionalLightMesh(DirectionalLightMesh&& other) noexcept;
	DirectionalLightMesh& operator= (DirectionalLightMesh&& other) noexcept;
	~DirectionalLightMesh() noexcept;
	void render() noexcept;

	static mat4f generateTransform(const vec3f& pos, const vec3f& dir, const vec3f& up) noexcept;

private:
	GLuint mVAO;
	GLuint mPosBuffer, mIndexBuffer;
};

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