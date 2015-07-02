#pragma once
#ifndef VOX_RENDERING_CAMERA_HPP
#define VOX_RENDERING_CAMERA_HPP

#include <sfz/Math.hpp>
#include <sfz/Geometry.hpp>



namespace vox {

using sfz::vec3;
using sfz::mat4;
using sfz::AABB;
using sfz::OBB;
using sfz::Plane;

struct Camera final {
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	vec3 mPos, mDir, mUp;
	float mVerticalFov, mAspectRatio, mNear, mFar;
	mat4 mViewMatrix, mProjMatrix;
	Plane mNearPlane, mFarPlane, mUpPlane, mDownPlane, mLeftPlane, mRightPlane;

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Camera(const vec3& position, const vec3& direction, const vec3& upVector,
	       float verticalFov, float aspectRatio, float near, float far) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void updateMatrices() noexcept;
	void updatePlanes() noexcept;
	bool isVisible(const AABB& aabb) const noexcept;
	bool isVisible(const OBB& obb) const noexcept;
	bool isVisible(const Camera& cam) const noexcept;
};

} // namespace vox


#endif