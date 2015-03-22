#pragma once
#ifndef VOX_RENDERING_CAMERA_HPP
#define VOX_RENDERING_CAMERA_HPP

#include <sfz/Math.hpp>
#include <sfz/Geometry.hpp>

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using sfz::vec3f;
using sfz::mat4f;
using sfz::AABB;

struct Camera final {
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	vec3f mPos, mDir, mUp;
	float mVerticalFov, mAspectRatio, mNear, mFar;
	mat4f mViewMatrix, mProjMatrix;

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Camera(const vec3f& position, const vec3f& direction, const vec3f& upVector,
	       float verticalFov, float aspectRatio, float near, float far) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void updateMatrices() noexcept;
	bool isVisible(const AABB& aabb) const noexcept;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif