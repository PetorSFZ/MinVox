#pragma once
#ifndef SFZ_GL_FONT_FONT_RENDERER_HPP
#define SFZ_GL_FONT_FONT_RENDERER_HPP

#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/Utils.hpp>
#include <sfz/gl/SpriteBatch.hpp>
#include <string>
#include <cstddef> // size_t
#include <cstdint> // uint8_t

//#include "rendering/FullscreenQuadObject.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

using std::size_t;
using std::uint8_t;

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

	void print(GLuint fbo, GLuint tex, float width, float height, const std::string& text, float x, float y, float size) noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	static const size_t FIRST_CHAR = 32; // inclusive
	static const size_t LAST_CHAR = 246; // inclusive
	static const size_t CHAR_COUNT = LAST_CHAR - FIRST_CHAR + 2; // 1 extra for unknown char

	std::string mFontPath;
	const float mFontSize;
	GLuint mFontRendererShader;

	//vox::FullscreenQuadObject mFullscreenQuad;
	GLuint mFontTexture;
	sfz::SpriteBatch mSpriteBatch;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif