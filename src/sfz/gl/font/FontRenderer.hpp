#pragma once
#ifndef SFZ_GL_FONT_FONT_RENDERER_HPP
#define SFZ_GL_FONT_FONT_RENDERER_HPP

#include <sfz/Math.hpp>
#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/SpriteBatch.hpp>
#include <sfz/gl/Alignment.hpp>
#include <cstddef> // size_t
#include <cstdint> // uint8_t
#include <memory>
#include <string>

#include <sfz/MSVC12HackON.hpp>

namespace gl {

using std::size_t;
using std::uint8_t;
using std::uint32_t;
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

	FontRenderer(const std::string& fontPath, uint32_t texWidth, uint32_t texHeight,
	             float fontSize, size_t numCharsPerBatch) noexcept;
	~FontRenderer() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void begin(vec2f cameraPosition, vec2f cameraDimensions) noexcept;

	/** @return The position to write the next char at. */
	float write(vec2f position, float size, const std::string& text) noexcept;

	void writeBitmapFont(vec2f position, vec2f dimensions) noexcept;

	void end(GLuint fbo, vec2f viewportDimensions, vec4f textColor) noexcept;

	float measureStringWidth(float size, const std::string& text) const noexcept;

	// Getters / setters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	inline HorizontalAlign horizontalAlign() const noexcept { return mHorizAlign; }
	inline VerticalAlign verticalAlign() const noexcept { return mVertAlign; }

	inline void horizontalAlign(HorizontalAlign align) noexcept { mHorizAlign = align; }
	inline void verticalAlign(VerticalAlign align) noexcept { mVertAlign = align; }

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	static const uint32_t FIRST_CHAR = 32; // inclusive
	static const uint32_t LAST_CHAR = 246; // inclusive
	static const uint32_t CHAR_COUNT = LAST_CHAR - FIRST_CHAR + 1;
	static const uint32_t UNKNOWN_CHAR = '?';

	const float mFontSize;
	const vec2f mPixelToUV;
	GLuint mFontTexture;
	void* const mPackedChars; // Type is implementation defined
	SpriteBatch mSpriteBatch;
	HorizontalAlign mHorizAlign = HorizontalAlign::LEFT;
	VerticalAlign mVertAlign = VerticalAlign::MIDDLE;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif