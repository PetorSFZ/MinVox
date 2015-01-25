#include "rendering/Camera.hpp"

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Camera::Camera()
{
	mPos = sfz::vec3f{-3, 1.2f, 0.2};
	mDir = sfz::vec3f{1, -0.1f, 0}.normalize();
	mUp = sfz::vec3f{0, 1, 0};
	mFov = 75;
	mViewMatrix = sfz::lookAt(mPos, mPos + mDir, mUp);
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void Camera::update()
{

	mViewMatrix = sfz::lookAt(mPos, mPos + mDir, mUp);
}

} // namespace vox