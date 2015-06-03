#include "rendering/Light.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

DirectionalLightMesh::DirectionalLightMesh() noexcept
{
	glGenBuffers(1, &mPosBuffer);
	glGenBuffers(1, &mIndexBuffer);
	glGenVertexArrays(1, &mVAO);
}

DirectionalLightMesh::DirectionalLightMesh(float fov, float near, float far) noexcept
{
	float nearOffs = std::tanf(fov/2.0f)*near;
	float farOffs = std::tanf(fov/2.0f)*far;
	const float positions[] = {
		// x, y, z
		// Near
		-nearOffs, -nearOffs, near, // 0, left-bottom-near
		nearOffs, -nearOffs, near,  // 1, right-bottom-near
		-nearOffs, nearOffs, near,  // 2, left-top-near
		nearOffs, nearOffs, near,   // 3, right-top-near

		// Far
		-farOffs, -farOffs, far, // 4, left-bottom-far
		farOffs, -farOffs, far,  // 5, right-bottom-far
		-farOffs, farOffs, far,  // 6, left-top-far
		farOffs, farOffs, far    // 7, right-top-far
	};
	const unsigned int indices[] = {
		// Back
		0, 2, 3,
		0, 3, 1,

		// Front
		4, 7, 6,
		4, 5, 7,

		// Left
		6, 0, 4,
		6, 2, 0,

		// Right
		3, 7, 1,
		1, 7, 5,

		// Top
		3, 2, 6,
		3, 6, 7,

		// Bottom
		0, 1, 4,
		4, 1, 5
	};

	// Buffer objects
	glGenBuffers(1, &mPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors likely caused by DirectionalLightMesh constructor." << std::endl;
	}
}

DirectionalLightMesh::DirectionalLightMesh(DirectionalLightMesh&& other) noexcept
:
	DirectionalLightMesh()
{
	std::swap(mPosBuffer, other.mPosBuffer);
	std::swap(mIndexBuffer, other.mIndexBuffer);
	std::swap(mVAO, other.mVAO);
}

DirectionalLightMesh& DirectionalLightMesh::operator= (DirectionalLightMesh&& other) noexcept
{
	std::swap(mPosBuffer, other.mPosBuffer);
	std::swap(mIndexBuffer, other.mIndexBuffer);
	std::swap(mVAO, other.mVAO);
	return *this;
}

DirectionalLightMesh::~DirectionalLightMesh() noexcept
{
	glDeleteBuffers(1, &mPosBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVAO);
}

void DirectionalLightMesh::render() noexcept
{
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
}

mat4f DirectionalLightMesh::generateTransform(const vec3f& pos, const vec3f& dir, const vec3f& up) noexcept
{
	vec4f transfForward = vec4f{dir[0], dir[1], dir[2], 0.0f}.normalize();
	vec4f transfUp = vec4f{up[0], up[1], up[2], 0.0f}.normalize();
	vec3f right = sfz::cross(up, dir).normalize();
	vec4f transfRight{right[0], right[1], right[2], 0.0f};

	mat4f temp;
	temp.setColumn(0, transfRight);
	temp.setColumn(1, transfUp);
	temp.setColumn(2, transfForward);
	temp.setColumn(3, vec4f{pos[0], pos[1], pos[2], 1.0f});
	return temp;
}

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