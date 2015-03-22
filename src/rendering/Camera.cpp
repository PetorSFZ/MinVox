#include "rendering/Camera.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Camera::Camera(const vec3f& position, const vec3f& direction, const vec3f& upVector,
               float verticalFov, float aspectRatio, float near, float far) noexcept
:
	mPos{position},
	mDir{direction},
	mUp{upVector},
	mVerticalFov{verticalFov},
	mAspectRatio{aspectRatio},
	mNear{near},
	mFar{far}
{
	sfz_assert_debug(near > 0);
	sfz_assert_debug(far > near);
	updateMatrices();
	updatePlanes();
}


// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void Camera::updateMatrices() noexcept
{
	mViewMatrix = sfz::lookAt(mPos, mPos + mDir, mUp);
	mProjMatrix = sfz::glPerspectiveProjectionMatrix(mVerticalFov, mAspectRatio, mNear, mFar);
}

void Camera::updatePlanes() noexcept
{
	const vec3f normDir = mDir.normalize();
	const vec3f normUpDir = (mUp - mUp.dot(normDir)*normDir).normalize();
	const vec3f normRightDir = sfz::cross(normDir, normUpDir).normalize();
	const float yHalfRadAngle = (mVerticalFov/2.0f) * sfz::g_DEG_TO_RAD_FLOAT;
	const float xHalfRadAngle = mAspectRatio * yHalfRadAngle;
	
	sfz_assert_debug(sfz::approxEqual(normDir.dot(normUpDir), 0.0f, 0.0001f));
	sfz_assert_debug(sfz::approxEqual(normDir.dot(normRightDir), 0.0f, 0.0001f));
	sfz_assert_debug(sfz::approxEqual(normUpDir.dot(normRightDir), 0.0f, 0.0001f));

	mNearPlane = Plane{-normDir, mPos + normDir*mNear};
	mFarPlane = Plane{normDir, mPos + normDir*mFar};
	
	vec3f upPlaneDir = sfz::rotationMatrix3(normRightDir, yHalfRadAngle) * normDir;
	vec3f upPlaneNormal = sfz::cross(upPlaneDir, -normRightDir).normalize();
	mUpPlane = Plane{upPlaneNormal, mPos};

	vec3f downPlaneDir = sfz::rotationMatrix3(-normRightDir, yHalfRadAngle) * normDir;
	vec3f downPlaneNormal = sfz::cross(downPlaneDir, normRightDir).normalize();
	mDownPlane = Plane{downPlaneNormal, mPos};

	vec3f rightPlaneDir = sfz::rotationMatrix3(-normUpDir, xHalfRadAngle) * normDir;
	vec3f rightPlaneNormal = sfz::cross(rightPlaneDir, normUpDir).normalize();
	mRightPlane = Plane{rightPlaneNormal, mPos};

	vec3f leftPlaneDir = sfz::rotationMatrix3(normUpDir, xHalfRadAngle) * normDir;
	vec3f leftPlaneNormal = sfz::cross(leftPlaneDir, -normUpDir).normalize();
	mLeftPlane = Plane{leftPlaneNormal, mPos};
}

bool Camera::isVisible(const AABB& aabb) const noexcept
{
	using sfz::belowPlane;
	if (!(belowPlane(mLeftPlane, aabb) && belowPlane(mRightPlane, aabb))) return false;
	if (!(belowPlane(mNearPlane, aabb) && belowPlane(mFarPlane, aabb))) return false;
	if (!(belowPlane(mUpPlane, aabb) && belowPlane(mDownPlane, aabb))) return false;
	return true;
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>