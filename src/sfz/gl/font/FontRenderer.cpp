#include "sfz/gl/font/FontRenderer.hpp"

#include "stb_rect_pack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

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

} // anonymous namespace

// FontRenderer: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

FontRenderer::FontRenderer(const std::string& fontPath, float fontSize) noexcept
:
	mFontPath{fontPath},
	mFontSize{fontSize},
	mFontRendererShader{compileFontRendererShaderProgram()}
{
	unsigned char* ttf_buffer = new unsigned char[1<<20];
	unsigned char* temp_bitmap = new unsigned char[512*512];
	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

	std::fread(ttf_buffer, 1, 1<<20, std::fopen(fontPath.c_str(), "rb"));
	stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
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

	mFullscreenQuad.render();

	// Cleanup
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
}

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>