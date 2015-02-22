#include "rendering/Camera.hpp"

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Camera::Camera()
{
	mPos = sfz::vec3f{-3.0f, 1.2f, 0.2f};
	mDir = sfz::vec3f{1.0f, -0.1f, 0.0f}.normalize();
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