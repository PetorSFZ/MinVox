#include "sfz/gl/font/FontRenderer.hpp"

#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

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

const char* VERTEX_SHADER = R"(
	#version 330

	in vec2 position;
	in vec2 texCoordIn;

	out vec2 texCoord;

	void main()
	{
		gl_Position = vec4(position, 0.0, 1.0);
		texCoord = texCoordIn;
	}
)";

const char* FRAGMENT_SHADER = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 texCoord;

	// Output
	out vec4 fragmentColor;

	// Uniforms
	uniform sampler2D uFontTexture;

	void main()
	{
		float color = texture(uFontTexture, texCoord).r;
		fragmentColor = vec4(vec3(color), 1.0);
	}
)";

GLuint compileFontRendererShaderProgram() noexcept
{
	GLuint vertexShader = gl::compileVertexShader(VERTEX_SHADER);
	GLuint fragmentShader = gl::compileFragmentShader(FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}

	return shaderProgram;
}

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
	mFontRendererShader{compileFontRendererShaderProgram()},
	mSpriteBatch{1000}
{
	unsigned char* temp_bitmap = new unsigned char[512*512];
	stbtt_bakedchar cdata[CHAR_COUNT];

	uint8_t* ttfBuffer = loadTTFBuffer(fontPath);
	stbtt_BakeFontBitmap(ttfBuffer,0, fontSize, temp_bitmap,512,512, FIRST_CHAR, CHAR_COUNT, cdata); // no guarantee this fits!
	delete[] ttfBuffer;


	glGenTextures(1, &mFontTexture);
	glBindTexture(GL_TEXTURE_2D, mFontTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

FontRenderer::~FontRenderer() noexcept
{
	glDeleteTextures(1, &mFontTexture);
}

// FontRenderer: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void FontRenderer::print(GLuint fbo, GLuint tex, float width, float height, const std::string& text, float x, float y, float size) noexcept
{
	// Save previous depth test state and then disable it
	GLboolean depthTestWasEnabled;
	glGetBooleanv(GL_DEPTH_TEST, &depthTestWasEnabled);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(mFontRendererShader);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // TODO: Debug
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO: Debug

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mFontTexture);
	gl::setUniform(mFontRendererShader, "uFontTexture", 0);

	//mFullscreenQuad.render();
	mSpriteBatch.begin();

	mSpriteBatch.draw(vec2f::ZERO(), vec2f{2.0f, 2.0f},  0.0f, sfz::TextureRegion{vec2f{0.0f, 0.0f}, vec2f{1.0f, 1.0f}});

	mSpriteBatch.draw(vec2f{-0.25f, -0.25f}, vec2f{0.5f, 0.5f}, 0.0f, sfz::TextureRegion{vec2f{0.0f, 0.0f}, vec2f{1.0f, 1.0f}});

	mSpriteBatch.draw(vec2f{0.25f, 0.25f}, vec2f{0.5f, 0.5f}, 0.0f, sfz::TextureRegion{vec2f{0.0f, 0.0f}, vec2f{0.5f, 0.5f}});

	mSpriteBatch.end(fbo, width, height, mFontTexture);

	// Cleanup
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
}

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>