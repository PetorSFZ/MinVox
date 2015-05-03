#include "sfz/gl/font/FontRenderer.hpp"

#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

#include <sfz/gl/Utils.hpp>
#include <new> // std::nothrow
#include <cstdio>
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

} // anonymous namespace

// FontRenderer: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

FontRenderer::FontRenderer(const std::string& fontPath, float fontSize) noexcept
:
	mFontPath{fontPath},
	mFontSize{fontSize},
	mSpriteBatch{1000, FONT_RENDERER_FRAGMENT_SHADER_SRC},
	mCharTexRegions{new (std::nothrow) TextureRegion[CHAR_COUNT]},
	mCharWidths{new (std::nothrow) float[CHAR_COUNT]}
{
	unsigned char* temp_bitmap = new unsigned char[512*512];
	stbtt_bakedchar cdata[CHAR_COUNT-1];

	uint8_t* ttfBuffer = loadTTFBuffer(fontPath);
	stbtt_BakeFontBitmap(ttfBuffer,0, fontSize, temp_bitmap,512,512, FIRST_CHAR, CHAR_COUNT-1, cdata); // no guarantee this fits!
	delete[] ttfBuffer;

	glGenTextures(1, &mFontTexture);
	glBindTexture(GL_TEXTURE_2D, mFontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Character arrays
	sfz_assert_debug(LAST_CHAR == (CHAR_COUNT-1));
	for (size_t i = 0; i < CHAR_COUNT-1; i++) {
		mCharTexRegions[i] = TextureRegion{
		                       vec2f{(float)cdata[i].x0/512.0f, (float)cdata[i].y0/512.0f},
		                       vec2f{(float)cdata[i].x1/512.0f, (float)cdata[i].y1/512.0f}};
		mCharWidths[i] = cdata[i].xadvance;
	}
	mCharTexRegions[LAST_CHAR] = mCharTexRegions[size_t(UNKNOWN_CHAR)-FIRST_CHAR];
	mCharWidths[LAST_CHAR] = mCharWidths[size_t(UNKNOWN_CHAR)-FIRST_CHAR];
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
	vec2f currentPos = position;

	for (unsigned char c : text) {
		size_t index = size_t(c) - FIRST_CHAR;
		if (index > LAST_CHAR) index = LAST_CHAR+1; // Location of unknown char
		TextureRegion& charRegion = mCharTexRegions[index];
		mSpriteBatch.draw(currentPos, vec2f{size, size}, charRegion);
		currentPos[0] += size;
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