#pragma once
#ifndef SFZ_GL_SPRITE_BATCH_HPP
#define SFZ_GL_SPRITE_BATCH_HPP

#include <sfz/Assert.hpp>
#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/Utils.hpp>

#include <cstddef> // size_t
#include <memory>

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

using std::size_t;
using sfz::vec2f;

// TextureRegion (for use with SpriteBatch)
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct TextureRegion final {
	vec2f mUVMin, mUVMax;
	inline TextureRegion(vec2f min, vec2f max) noexcept : mUVMin{min}, mUVMax{max} { };
};

// SpriteBatch
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class SpriteBatch final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	SpriteBatch() = delete;
	SpriteBatch(const SpriteBatch&) = delete;
	SpriteBatch& operator= (const SpriteBatch&) = delete;

	SpriteBatch(size_t capacity) noexcept;
	//SpriteBatch(SpriteBatch&& other) noexcept;
	//SpriteBatch& operator= (SpriteBatch&& other) noexcept;
	~SpriteBatch() noexcept;

	// Public interface
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void begin() noexcept;

	void draw(vec2f position, vec2f dimensions, float angleRads,
	          const TextureRegion& texRegion) noexcept;

	void end(GLuint fbo, float fbWidth, float fbHeight, GLuint texture) noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	size_t mCapacity;
	size_t mCurrentDrawCount;

	GLuint mShader;
	GLuint mVAO;
	GLuint mVertexBuffer, mIndexBuffer, mPosBuffer, mDimBuffer, mAngleBuffer, mUVBuffer;
	std::unique_ptr<vec2f[]> mPosArray;
	std::unique_ptr<vec2f[]> mDimArray;
	std::unique_ptr<float[]> mAngleArray;
	std::unique_ptr<vec4f[]> mUVArray;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif