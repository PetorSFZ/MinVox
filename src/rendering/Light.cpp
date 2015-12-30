#include "rendering/Light.hpp"



namespace vox {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

vec3 generateUpVector(const vec3& dir) noexcept
{
	static const vec3 UP{0.0f, 1.0f, 0.0};
	vec3 axis;
	if (dir != UP) axis = sfz::cross(dir, UP);
	else axis = sfz::cross(dir, UP + vec3{0.1f, 0.0f, 0.0f});
	sfz::mat3 rotation = sfz::rotationMatrix3(axis, 90.0f*sfz::DEG_TO_RAD());
	return rotation * dir;
}

} // anonymous namespace

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

mat4 DirectionalLightMesh::generateTransform(const vec3& pos, const vec3& dir, const vec3& up) noexcept
{
	vec4 transfForward = sfz::normalize(vec4{dir[0], dir[1], dir[2], 0.0f});
	vec4 transfUp = sfz::normalize(vec4{up[0], up[1], up[2], 0.0f});
	vec3 right = sfz::normalize(sfz::cross(up, dir));
	vec4 transfRight{right[0], right[1], right[2], 0.0f};

	mat4 temp;
	temp.setColumn(0, transfRight);
	temp.setColumn(1, transfUp);
	temp.setColumn(2, transfForward);
	temp.setColumn(3, vec4{pos[0], pos[1], pos[2], 1.0f});
	return temp;
}

// DirectionalLight: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

DirectionalLight::DirectionalLight(const vec3& position, const vec3& direction, const vec3& upVector,
                                   float verticalFov, float near, float far) noexcept
:
	mCam{position, direction, upVector, verticalFov, 1.0f, near, far}
{
	sfz_assert_debug(near > 0);
	sfz_assert_debug(far > near);
	update();
}

	
DirectionalLight::DirectionalLight(const vec3& pos, const vec3& dir, float near, float range,
                                   const vec3& color) noexcept
:
	mCam{pos, dir, generateUpVector(dir), 45.0f, 1.0f, near, range},
	mRange{range},
	mColor(color)
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

mat4 DirectionalLight::lightMatrix(const mat4& inverseViewMatrix) const noexcept
{
	static const mat4 translScale = sfz::translationMatrix(0.5f, 0.5f, 0.5f)
	                               * sfz::scalingMatrix4(0.5f);
	return translScale * mCam.mProjMatrix * mCam.mViewMatrix * inverseViewMatrix;
}

} // namespace vox

