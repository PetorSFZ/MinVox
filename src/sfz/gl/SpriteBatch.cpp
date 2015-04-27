#include "sfz/gl/SpriteBatch.hpp"

#include <new> // std::nothrow

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

// Anonymous: Shaders
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const char* VERTEX_SHADER = R"(
	#version 330

	// Input
	in vec2 vertexIn;
	in vec2 positionIn;
	in vec2 dimensionsIn;
	in vec2 uvCoordIn;

	// Ouput
	out vec2 uvCoord;

	void main()
	{
		gl_Position = vec4(vertexIn, 0.0, 1.0);
		uvCoord = uvCoordIn;
	}
)";

const char* FRAGMENT_SHADER = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 uvCoord;

	// Output
	out vec4 fragmentColor;

	// Uniforms
	uniform sampler2D uTexture;

	void main()
	{
		fragmentColor = vec4(texture(uTexture, uvCoord).rgb, 1.0);
	}
)";

GLuint compileSpriteBatchShaderProgram() noexcept
{
	GLuint vertexShader = gl::compileVertexShader(VERTEX_SHADER);
	GLuint fragmentShader = gl::compileFragmentShader(FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "vertexIn");
	glBindAttribLocation(shaderProgram, 1, "positionIn");
	glBindAttribLocation(shaderProgram, 2, "dimensionsIn");
	glBindAttribLocation(shaderProgram, 3, "uvCoordIn");
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}

	return shaderProgram;
}

} // anonymous namespace

// SpriteBatch: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SpriteBatch::SpriteBatch(size_t capacity) noexcept
:
	mCapacity{capacity},
	mCurrentDrawCount{0},
	mShader{compileSpriteBatchShaderProgram()},
	mPosArray{new (std::nothrow) vec2f[mCapacity]},
	mDimArray{new (std::nothrow) vec2f[mCapacity]},
	mUVArray{new (std::nothrow) vec2f[mCapacity*4]}
{
	static_assert(sizeof(vec2f) == sizeof(float)*2, "vec2f is padded");

	// Vertex buffer
	const float vertices[] = {
		-0.5f, -0.5f, // left bottom
		0.5f, -0.5f,  // right bottom
		-0.5f, 0.5f,  // left top
		0.5f, 0.5f    // right top
	};
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position buffer (null now, to be updated when rendering batch)
	glGenBuffers(1, &mPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f)*mCapacity, NULL, GL_STREAM_DRAW);

	// Dimensions buffer (null now, to be updated when rendering batch)
	glGenBuffers(1, &mDimBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mDimBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f)*mCapacity, NULL, GL_STREAM_DRAW);

	// UV buffer (null now, to be updated when rendering batch)
	glGenBuffers(1, &mUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2f)*mCapacity*4, NULL, GL_STREAM_DRAW);

	// Index buffer
	const unsigned int indices[] = {
		0, 1, 2,
		1, 3, 2
	};
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glVertexAttribPointer(0, 2 /* size */, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glVertexAttribPointer(1, 2 /* size */, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, mDimBuffer);
	glVertexAttribPointer(2, 2 /* size */, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glVertexAttribPointer(3, 2 /*size*/, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by SpriteBatch constructor" << std::endl;
	}
}

SpriteBatch::~SpriteBatch() noexcept
{
	glDeleteProgram(mShader);
	glDeleteBuffers(1, &mVertexBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteBuffers(1, &mPosBuffer);
	glDeleteBuffers(1, &mDimBuffer);
	glDeleteBuffers(1, &mUVBuffer);
	glDeleteVertexArrays(1, &mVAO);
}

// SpriteBatch: Public interface
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SpriteBatch::begin() noexcept
{
	mCurrentDrawCount = 0;
}

void SpriteBatch::draw(vec2f position, vec2f dimensions, float angleRads,
					   const TextureRegion& texRegion) noexcept
{
	// Setting position och dimensions arrays
	mPosArray[mCurrentDrawCount] = position;
	mDimArray[mCurrentDrawCount] = dimensions;

	// Setting uv array
	const size_t uvBaseIndex = mCurrentDrawCount*4;
	mUVArray[uvBaseIndex] = texRegion.mUVMin;
	mUVArray[uvBaseIndex+1] = vec2f{texRegion.mUVMax[0], texRegion.mUVMin[1]};
	mUVArray[uvBaseIndex+2] = vec2f{texRegion.mUVMin[0], texRegion.mUVMax[1]};
	mUVArray[uvBaseIndex+3] = texRegion.mUVMax;

	// Incrementing current draw count
	mCurrentDrawCount++;
	sfz_assert_debug(mCurrentDrawCount <= mCapacity);
}

void SpriteBatch::end(GLuint fbo, float fbWidth, float fbHeight, GLuint texture) noexcept
{
	// Save previous depth test state and then disable it
	GLboolean depthTestWasEnabled;
	glGetBooleanv(GL_DEPTH_TEST, &depthTestWasEnabled);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(mShader);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, fbWidth, fbHeight);

	// Uniforms
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	gl::setUniform(mShader, "uTexture", 0);

	// Reuse same vertices for each draw instance
	glVertexAttribDivisor(0, 0);
	// One position, dimension, and uv coords for each instance
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, mCurrentDrawCount);

	// Restore previous depth test state
	if (depthTestWasEnabled) glEnable(GL_DEPTH_TEST);

	// Cleanup
	glUseProgram(0); // TODO: Store previous program
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // TODO: Store previous framebuffer
}


} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>