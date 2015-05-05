#include "sfz/gl/font/FontRenderer.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#include <sfz/gl/Utils.hpp>
#include <new> // std::nothrow
#include <cstdio>
#include <cstdlib> // malloc
#include <cstring> // std::memcpy
#include <iostream> // std::cerr
#include <exception> // std::terminate

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

// Anonymous: Shaders
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const char* FONT_RENDERER_FRAGMENT_SHADER_SRC = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 uvCoord;

	// Output
	out vec4 fragmentColor;

	// Uniforms
	uniform sampler2D uTexture;
	uniform vec4 uTextColor;

	void main()
	{
		fragmentColor = uTextColor * texture(uTexture, uvCoord).r;
	}
)";

uint8_t* loadTTFBuffer(const std::string& path) noexcept
{
	const size_t MAX_TTF_BUFFER_SIZE = 1<<22; // 4 MiB
	uint8_t* buffer = new (std::nothrow) uint8_t[MAX_TTF_BUFFER_SIZE];

	std::FILE* ttfFile = std::fopen(path.c_str(), "rb");
	if (ttfFile == NULL) {
		std::cerr << "Couldn't open TTF file at: " << path << std::endl;
		std::terminate();
	}

	size_t readCount = std::fread(buffer, sizeof(uint8_t), MAX_TTF_BUFFER_SIZE, ttfFile);
	if (readCount == 0) {
		std::cerr << "Loaded no bytes from TTF file at: " << path << std::endl;
		std::terminate();
	}

	std::fclose(ttfFile);
	return buffer;
}

void flipBitmapFontTexture(uint8_t* const bitmapFont, size_t w, size_t h) noexcept
{
	const size_t pixelCount = w * h;
	uint8_t* const tempBuffer = new (std::nothrow) uint8_t[pixelCount];

	uint8_t* readPtr = bitmapFont; // Reads from bitmap font, starting value is first pixel.
	uint8_t* writePtr = tempBuffer + pixelCount; // Starting value is first pixel outside range

	// Copy pixels from bitmap font to buffer flipping the rows along the way.
	while (writePtr > tempBuffer) {
		writePtr = writePtr - w; // Move writePtr back one image row
		std::memcpy(writePtr, readPtr, w); // Copy one image row to temp buffer
		readPtr = readPtr + w;
	}

	// Copy pixels back from temp buffer into the bitmapFont
	std::memcpy(bitmapFont, tempBuffer, pixelCount);
	delete[] tempBuffer;
}

TextureRegion calculateTextureRegion(const stbtt_bakedchar& c, float w, float h) noexcept
{
	vec2f minTemp{static_cast<float>(c.x0)/w, (h - static_cast<float>(c.y1))/h};
	vec2f maxTemp{static_cast<float>(c.x1)/w, (h - static_cast<float>(c.y0))/h};
	return TextureRegion{minTemp, maxTemp};
}

} // anonymous namespace

// FontRenderer: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

FontRenderer::FontRenderer(const std::string& fontPath, size_t numCharsPerBatch, float fontSize) noexcept
:
	mFontPath{fontPath},
	mFontSize{fontSize},
	mTexWidth{1024},
	mTexHeight{1024},
	mSpriteBatch{numCharsPerBatch, FONT_RENDERER_FRAGMENT_SHADER_SRC},
	mPackedChars{new (std::nothrow) stbtt_packedchar[CHAR_COUNT]}
{
	uint8_t* tempBitmap = new uint8_t[mTexWidth*mTexHeight];

	stbtt_pack_context packContext;
	if(stbtt_PackBegin(&packContext, tempBitmap, mTexWidth, mTexHeight, 0, 1, NULL) == 0) {
		std::cerr << "FontRenderer: Couldn't stbtt_PackBegin()" << std::endl;
		std::terminate();
	}

	stbtt_PackSetOversampling(&packContext, 2, 2);

	uint8_t* ttfBuffer = loadTTFBuffer(fontPath);

	stbtt_PackFontRange(&packContext, ttfBuffer, 0, mFontSize, FIRST_CHAR, CHAR_COUNT,
	                    reinterpret_cast<stbtt_packedchar*>(mPackedChars));

	stbtt_PackEnd(&packContext);
	delete[] ttfBuffer;

	//flipBitmapFontTexture(tempBitmap, width, height);

	glGenTextures(1, &mFontTexture);
	glBindTexture(GL_TEXTURE_2D, mFontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, mTexWidth, mTexHeight, 0, GL_RED, GL_UNSIGNED_BYTE,
	             tempBitmap);
	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Enable anisotropic filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
	delete[] tempBitmap;
}

FontRenderer::~FontRenderer() noexcept
{
	glDeleteTextures(1, &mFontTexture);
	delete[] reinterpret_cast<stbtt_packedchar* const>(mPackedChars);
}

// FontRenderer: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void FontRenderer::begin(vec2f cameraPosition, vec2f cameraDimensions) noexcept
{
	mSpriteBatch.begin(cameraPosition, cameraDimensions);
}

void FontRenderer::write(vec2f position, float size, const std::string& text) noexcept
{
	stbtt_aligned_quad quad;
	vec2f advPos = position;
	vec2f pos, dim;
	TextureRegion texRegion;
	for (unsigned char c : text) {
		int codepoint = c - FIRST_CHAR;
		if (codepoint < 0 || (int)LAST_CHAR < codepoint) codepoint = UNKNOWN_CHAR - FIRST_CHAR;
		stbtt_GetPackedQuad(reinterpret_cast<stbtt_packedchar*>(mPackedChars), mTexWidth,
		                    mTexHeight, codepoint, &advPos[0], &advPos[1], &quad, false);

		dim[0] = quad.x1 - quad.x0;
		dim[1] = quad.y1 - quad.y0;
		pos[0] = (quad.x0 + quad.x1) / 2.0f;
		pos[1] = ((quad.y0 + quad.y1) / 2.0f) + dim[1];
		texRegion.mUVMin[0] = quad.s0;
		texRegion.mUVMin[1] = quad.t1;
		texRegion.mUVMax[0] = quad.s1;
		texRegion.mUVMax[1] = quad.t0;

		mSpriteBatch.draw(pos, dim, texRegion);
	}
}

void FontRenderer::writeBitmapFont(vec2f position, vec2f dimensions) noexcept
{
	mSpriteBatch.draw(position, dimensions, TextureRegion{vec2f{0.0f, 0.0f}, vec2f{1.0f, 1.0f}});
}

void FontRenderer::end(GLuint fbo, vec2f viewportDimensions, vec4f textColor) noexcept
{
	glUseProgram(mSpriteBatch.shaderProgram());
	gl::setUniform(mSpriteBatch.shaderProgram(), "uTextColor", textColor);
	mSpriteBatch.end(fbo, viewportDimensions, mFontTexture);
}

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>