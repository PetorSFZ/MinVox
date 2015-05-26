#include "rendering/Light.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// DirectionalLight: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

DirectionalLight::DirectionalLight(const vec3f& position, const vec3f& direction, const vec3f& upVector,
                                   float verticalFov, float near, float far) noexcept
:
	mCam{position, direction, upVector, verticalFov, 1.0f, near, far}
{
	sfz_assert_debug(near > 0);
	sfz_assert_debug(far > near);
	update();
}

	
DirectionalLight::DirectionalLight(const vec3f& pos, const vec3f& dir, float near, float range,
                                   const vec3f& color) noexcept
:
	mCam{pos, dir, vec3f{0.0f, 1.0f, 0.0f}, 45.0f, 1.0f, near, range},
	mRange{range},
	mColor{color}
{
	sfz_assert_debug(range > 0);
	update();
}

// DirectionalLight: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void DirectionalLight::update() noexcept
{
	mCam.mFar = mRange;
	mCam.updateMatrices();
	mCam.updatePlanes();
}

mat4f DirectionalLight::lightMatrix(const mat4f& inverseViewMatrix) const noexcept
{
	static const mat4f translScale = sfz::translationMatrix(0.5f, 0.5f, 0.5f)
	                               * sfz::scalingMatrix4(0.5f);
	return translScale * mCam.mProjMatrix * mCam.mViewMatrix * inverseViewMatrix;
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>