#pragma once
#ifndef VOX_RENDERING_LIGHT_HPP
#define VOX_RENDERING_LIGHT_HPP

#include <sfz/Math.hpp>
#include "rendering/Camera.hpp"
#include "sfz/GL.hpp"



namespace vox {

using sfz::vec3;
using sfz::vec4;
using sfz::mat4;

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

	static mat4 generateTransform(const vec3& pos, const vec3& dir, const vec3& up) noexcept;

private:
	GLuint mVAO;
	GLuint mPosBuffer, mIndexBuffer;
};

struct DirectionalLight final {
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Camera mCam;
	float mRange;
	vec3 mColor;

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	DirectionalLight(const vec3& position, const vec3& direction, const vec3& upVector,
	                 float verticalFov, float near, float far) noexcept;

	DirectionalLight(const vec3& pos, const vec3& dir, float near, float range,
	                 const vec3& color) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update() noexcept;
	mat4 lightMatrix(const mat4& inverseViewMatrix) const noexcept;
};

} // namespace vox


#endif