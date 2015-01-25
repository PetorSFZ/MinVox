#pragma once
#ifndef VOX_RENDERING_CAMERA_HPP
#define VOX_RENDERING_CAMERA_HPP

#include <sfz/Math.hpp>

namespace vox {

struct Camera final {

	sfz::vec3f mPos, mDir, mUp;
	float mFov;
	sfz::mat4f mViewMatrix;

	Camera();

	void update();
};

} // namespace vox

#endif