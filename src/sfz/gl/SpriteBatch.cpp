#include "sfz/gl/SpriteBatch.hpp"

#include <new> // std::nothrow

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

// SpriteBatch: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SpriteBatch::SpriteBatch(size_t capacity) noexcept
:
	mCapacity{capacity},
	mCurrentDrawCount{0},
	mPosArray{new (std::nothrow) vec2f[mCapacity]},
	mDimArray{new (std::nothrow) vec2f[mCapacity]},
	mUVArray{new (std::nothrow) vec2f[mCapacity*4]}
{
	static_assert(sizeof(vec2f) == sizeof(float)*2, "vec2f is padded");

	// Vertex Array Object
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	// Vertex buffer
	const float vertices[] = {
		-0.5f, -0.5f, // left bottom
		0.5f, -0.5f,  // right bottom
		-0.5f, 0.5f,  // left top
		0.5f, 0.5f    // right top
	};
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Index buffer
	const unsigned int indices[] = {
		0, 1, 2,
		1, 3, 2
	};
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position buffer (null now, to be updated when rendering batch)
	glGenBuffers(1, &mPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f)*mCapacity, NULL, GL_STREAM_DRAW);

	// Dimensions buffer (null now, to be updated when rendering batch)
	glGenBuffers(1, &mDimBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mDimBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f)*mCapacity, NULL, GL_STREAM_DRAW);

	// UV buffer (null now, to be updated when rendering batch)
	glGenBuffers(1, &mUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f)*4*mCapacity, NULL, GL_STREAM_DRAW);
}

SpriteBatch::~SpriteBatch() noexcept
{
	glDeleteBuffers(1, &mVertexBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteBuffers(1, &mPosBuffer);
	glDeleteBuffers(1, &mDimBuffer);
	glDeleteBuffers(1, &mUVBuffer);
	glDeleteVertexArrays(1, &mVAO);
}

// SpriteBatch: Public interface
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SpriteBatch::begin() noexcept
{
	mCurrentDrawCount = 0;
}

void SpriteBatch::draw(vec2f position, vec2f dimensions, float angleRads,
					   const TextureRegion& texRegion) noexcept
{
	// Setting position och dimensions arrays
	mPosArray[mCurrentDrawCount] = position;
	mDimArray[mCurrentDrawCount] = dimensions;

	// Setting uv array
	const size_t uvBaseIndex = mCurrentDrawCount*4;
	mUVArray[uvBaseIndex] = texRegion.mUVMin;
	mUVArray[uvBaseIndex+1] = vec2f{texRegion.mUVMax[0], texRegion.mUVMin[1]};
	mUVArray[uvBaseIndex+2] = vec2f{texRegion.mUVMin[0], texRegion.mUVMax[1]};
	mUVArray[uvBaseIndex+3] = texRegion.mUVMax;

	// Incrementing current draw count
	mCurrentDrawCount++;
	sfz_assert_debug(mCurrentDrawCount <= mCapacity);
}

void SpriteBatch::end() noexcept
{
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, mCurrentDrawCount);
}


} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>