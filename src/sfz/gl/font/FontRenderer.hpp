#pragma once
#ifndef SFZ_GL_FONT_FONT_RENDERER_HPP
#define SFZ_GL_FONT_FONT_RENDERER_HPP

#include <sfz/Math.hpp>
#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/Utils.hpp>
#include <sfz/gl/SpriteBatch.hpp>
#include <string>
#include <cstddef> // size_t
#include <cstdint> // uint8_t

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

using std::size_t;
using std::uint8_t;
using sfz::vec2f;
using sfz::vec4f;

// FontRenderer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class FontRenderer final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	FontRenderer() = delete;
	FontRenderer(const FontRenderer&) = delete;
	FontRenderer(FontRenderer&&) = delete;
	FontRenderer& operator= (const FontRenderer&) = delete;
	FontRenderer& operator= (FontRenderer&&) = delete;

	FontRenderer(const std::string& fontPath, float fontSize) noexcept;
	~FontRenderer() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void begin(vec2f cameraPosition, vec2f cameraDimensions, vec4f textColor) noexcept;

	void write(vec2f position, float size, const std::string& text) noexcept;

	void writeBitmapFont(vec2f position, vec2f dimensions) noexcept;

	void end(GLuint fbo, vec2f viewportDimensions) noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	static const size_t FIRST_CHAR = 32; // inclusive
	static const size_t LAST_CHAR = 246; // inclusive
	static const size_t CHAR_COUNT = LAST_CHAR - FIRST_CHAR + 2; // 1 extra for unknown char

	std::string mFontPath;
	const float mFontSize;

	GLuint mFontTexture;
	sfz::SpriteBatch mSpriteBatch;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif