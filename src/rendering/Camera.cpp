#include "rendering/Camera.hpp"

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

Camera::Camera()
{
	mPos = sfz::vec3f{0, 0, 0};
	mDir = sfz::vec3f{0, 0, 1};
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