#pragma once
#ifndef SFZ_GL_SPRITE_BATCH_HPP
#define SFZ_GL_SPRITE_BATCH_HPP

#include <sfz/Assert.hpp>
#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/Utils.hpp>

#include <cstddef> // size_t

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

using std::size_t;

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

	void draw() noexcept;

	void end() noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint mVAO;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif