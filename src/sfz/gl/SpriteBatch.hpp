#pragma once
#ifndef SFZ_GL_SPRITE_BATCH_HPP
#define SFZ_GL_SPRITE_BATCH_HPP

#include <sfz/Math.hpp>
#include <sfz/gl/OpenGL.hpp>

#include <cstddef> // size_t
#include <memory>

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

using std::size_t;

// TextureRegion (for use with SpriteBatch)
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

struct TextureRegion final {
	vec2f mUVMin, mUVMax;
	inline TextureRegion() noexcept = default;
	inline TextureRegion(vec2f min, vec2f max) noexcept : mUVMin{min}, mUVMax{max} { };
	inline vec2f dimensions() const noexcept { return mUVMax - mUVMin; }
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

	/**
	 * @brief Creates a SpriteBatch with the specified capacity.
	 */
	SpriteBatch(size_t capacity) noexcept;
	/**
	 * @brief Creates a SpriteBatch with a custom fragment shader.
	 * The fragment shader needs the following declarations:
	 * in vec2 uvCoord;
	 * out vec4 fragmentColor
	 * uniform sampler2D uTexture;
	 */
	SpriteBatch(size_t capacity, const char* fragmentShaderSrc) noexcept;
	SpriteBatch(SpriteBatch&& other) noexcept;
	SpriteBatch& operator= (SpriteBatch&& other) noexcept;
	~SpriteBatch() noexcept;

	// Public interface
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void begin(vec2f cameraPosition, vec2f cameraDimensions) noexcept;

	void draw(vec2f position, vec2f dimensions, const TextureRegion& texRegion) noexcept;

	void draw(vec2f position, vec2f dimensions, float angleRads,
	          const TextureRegion& texRegion) noexcept;

	void end(GLuint fbo, vec2f viewportDimensions, GLuint texture) noexcept;

	inline GLuint shaderProgram() const noexcept { return mShader; }

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	size_t mCapacity;
	size_t mCurrentDrawCount;
	mat3f mCamProj;

	GLuint mShader;
	GLuint mVAO;
	GLuint mVertexBuffer, mIndexBuffer, mTransformBuffer, mUVBuffer;
	std::unique_ptr<mat3f[]> mTransformArray;
	std::unique_ptr<vec4f[]> mUVArray;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif