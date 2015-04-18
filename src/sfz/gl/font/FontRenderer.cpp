#include "sfz/gl/font/FontRenderer.hpp"

#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

// FontRenderer: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

FontRenderer::FontRenderer(const std::string& fontPath, float fontSize, float width, float height) noexcept
:
	mFontPath{fontPath},
	mFontSize{fontSize},
	mWidth{width},
	mHeight{height}
{

}

FontRenderer::~FontRenderer() noexcept
{

}

// FontRenderer: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint FontRenderer::print(GLuint baseTex, const std::string& text,
                           float x, float y, float size) noexcept
{
	return 0; // TODO: Implement
}

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>