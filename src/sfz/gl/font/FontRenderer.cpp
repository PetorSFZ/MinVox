#include "sfz/gl/font/FontRenderer.hpp"

#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#include <sfz/gl/Utils.hpp>
#include <new> // std::nothrow
#include <cstdio>
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

	std::FILE* ttfFile = fopen(path.c_str(), "rb");
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
	mSpriteBatch{numCharsPerBatch, FONT_RENDERER_FRAGMENT_SHADER_SRC},
	mCharTexRegions{new (std::nothrow) TextureRegion[CHAR_COUNT]},
	mCharWidths{new (std::nothrow) float[CHAR_COUNT]},
	mCharOffsets{new (std::nothrow) float[CHAR_COUNT]}
{
	uint8_t* temp_bitmap = new uint8_t[512*512];
	stbtt_bakedchar cdata[CHAR_COUNT-1];

	uint8_t* ttfBuffer = loadTTFBuffer(fontPath);
	stbtt_BakeFontBitmap(ttfBuffer,0, fontSize, temp_bitmap,512,512, FIRST_CHAR, CHAR_COUNT-1, cdata); // no guarantee this fits!
	delete[] ttfBuffer;

	flipBitmapFontTexture(temp_bitmap, 512, 512);

	glGenTextures(1, &mFontTexture);
	glBindTexture(GL_TEXTURE_2D, mFontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Character arrays
	sfz_assert_debug(LAST_CHAR == (CHAR_COUNT-1));
	for (size_t i = 0; i < CHAR_COUNT-1; i++) {
		mCharTexRegions[i] = calculateTextureRegion(cdata[i], 512.0f, 512.0f);
		mCharWidths[i] = cdata[i].xadvance;
		mCharOffsets[i] = cdata[i].xoff;
	}
	mCharTexRegions[LAST_CHAR] = mCharTexRegions[size_t(UNKNOWN_CHAR)-FIRST_CHAR];
	mCharWidths[LAST_CHAR] = mCharWidths[size_t(UNKNOWN_CHAR)-FIRST_CHAR];
	mCharOffsets[LAST_CHAR] = mCharOffsets[size_t(UNKNOWN_CHAR)-FIRST_CHAR];
}

FontRenderer::~FontRenderer() noexcept
{
	glDeleteTextures(1, &mFontTexture);
}

// FontRenderer: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void FontRenderer::begin(vec2f cameraPosition, vec2f cameraDimensions) noexcept
{
	mSpriteBatch.begin(cameraPosition, cameraDimensions);
}

void FontRenderer::write(vec2f position, float size, const std::string& text) noexcept
{
	float scale = size / mFontSize;
	vec2f currentPos = position;

	for (unsigned char c : text) {
		size_t index = size_t(c) - FIRST_CHAR;
		if (index > LAST_CHAR) index = LAST_CHAR+1; // Location of unknown char
		TextureRegion& charRegion = mCharTexRegions[index];
		float charWidth = scale*mCharWidths[index];
		float charOffset = scale*mCharOffsets[index];
		mSpriteBatch.draw(currentPos, vec2f{charWidth, size}, charRegion);
		currentPos[0] += charWidth + charOffset;
	}

	//mSpriteBatch.draw(???, vec2f{???, size}, ???);
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