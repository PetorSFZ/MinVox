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
	updateMatrices();
}


// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void Camera::updateMatrices() noexcept
{
	mViewMatrix = sfz::lookAt(mPos, mPos + mDir, mUp);
	mProjMatrix = sfz::glPerspectiveProjectionMatrix(mVerticalFov, mAspectRatio, mNear, mFar);
}

bool Camera::isVisible(const AABB& aabb) const noexcept
{
	// TODO: Proper implementation to enable view frustrum culling!
	return true;
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>