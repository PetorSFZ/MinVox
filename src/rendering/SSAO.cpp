#include "rendering/SSAO.hpp"

#include "sfz/MSVC12HackON.hpp"

namespace vox {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

// Common vertex shader
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

// SSAO shader
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const char* SSAO_FRAGMENT_SHADER = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 texCoord;

	// Output
	out vec4 occlusionOut;

	// Constants
	const int MAX_KERNEL_SIZE = 256;

	// Uniforms
	uniform sampler2D uColorTexture;
	uniform sampler2D uDepthTexture;
	uniform sampler2D uNormalTexture;
	uniform sampler2D uPositionTexture;
	uniform sampler2D uNoiseTexture;
		
	uniform int uKernelSize;
	uniform vec3 uKernel[MAX_KERNEL_SIZE];
	uniform mat4 uProjectionMatrix;

	uniform vec2 uNoiseTexCoordScale;
	uniform float uRadius;

	float linearizeDepth(float depth)
	{
		/*float near = 0.5; // camera z-near
		float far = 1000.0; // camera z-far
		return (2.0 * near) / (far + near - (depth*(far-near)));*/
			
		// http://stackoverflow.com/questions/7777913/how-to-render-depth-linearly-in-modern-opengl-with-gl-fragcoord-z-in-fragment-sh/16597492#16597492
		vec4 unprojected = vec4(0, 0, depth * 2.0 - 1.0, 1.0);
		vec4 projected = inverse(uProjectionMatrix) * unprojected;
		projected /= projected.w;
		return projected.z;
	}

	float vsPosToDepth(vec3 vsPos)
	{
		float depth = vsPos.z;
		//if (depth >= 0) depth = -1000000.0;
		return depth;
	}

	vec2 texCoordFromVSPos(vec3 vsPos)
	{
		vec4 offset = uProjectionMatrix * vec4(vsPos, 1.0);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;
		return offset.xy;
	}

	void main()
	{
		vec4 color = texture(uColorTexture, texCoord);
		vec3 normal = normalize(texture(uNormalTexture, texCoord).rgb);
		vec3 vsPos = texture(uPositionTexture, texCoord).rgb;
		float depth = vsPosToDepth(vsPos);
		//float depth = linearizeDepth(texture(depthTexture, texCoord).r);

		//vec3 noiseVec = vec3(0.0, 1.0, 0); // Note, should really come from tiled noise texture.
		vec3 noiseVec = texture(uNoiseTexture, texCoord * uNoiseTexCoordScale).xyz;

		// http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
		vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 kernelRot = mat3(tangent, bitangent, normal);

		float occlusion = 0.0;
		for (int i = 0; i < uKernelSize; i++) {
			vec3 samplePos = vsPos + uRadius * (kernelRot * uKernel[i]);
			vec2 sampleTexCoord = texCoordFromVSPos(samplePos);
			float sampleDepth = vsPosToDepth(texture(uPositionTexture, sampleTexCoord).xyz);

			//float rangeCheck = abs(vsPos.z - sampleDepth) < uRadius ? 1.0 : 0.0;
			occlusion += (sampleDepth <= samplePos.z ? 1.0 : 0.0);// * rangeCheck;
		}
		occlusion /= uKernelSize;
		occlusion = pow(occlusion, 2.0);

		occlusionOut = vec4(occlusion, 0.0, 0.0, 1.0);
	}
)";

// SSAO blur shader
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const char* SSAO_BLUR_FRAGMENT_SHADER = R"(
	#version 330

	precision highp float; // required by GLSL spec Sect 4.5.3

	// Input
	in vec2 texCoord;
	
	// Output
	out vec4 fragmentColor;

	// Uniforms
	uniform sampler2D uOcclusionTexture;

	void main()
	{
		float occlusion = texture(uOcclusionTexture, texCoord).r;
		fragmentColor = vec4(vec3(occlusion), 1.0);
	}
)";

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GLuint compileSSAOShaderProgram() noexcept
{
	GLuint vertexShader = gl::compileVertexShader(VERTEX_SHADER);
	GLuint fragmentShader = gl::compileFragmentShader(SSAO_FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoordIn");
	glBindFragDataLocation(shaderProgram, 0, "occlusionOut");

	gl::linkProgram(shaderProgram);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors caused by shader compiling & linking." << std::endl;
	}

	return shaderProgram;
}

GLuint compileBlurShaderProgram() noexcept
{
	GLuint vertexShader = gl::compileVertexShader(VERTEX_SHADER);
	GLuint fragmentShader = gl::compileFragmentShader(SSAO_BLUR_FRAGMENT_SHADER);

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

// Lerp function from wikipedia: http://en.wikipedia.org/wiki/Lerp_%28computing%29
// Precise method which guarantees v = v1 when t = 1.
float lerp(float v0, float v1, float t) noexcept
{
	return (1-t)*v0 + t*v1;
}

vector<vec3f> generateKernel(size_t kernelSize) noexcept
{
	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr1{-1.0f, 1.0f};
	std::uniform_real_distribution<float> distr2{0.0f, 1.0f};

	vector<vec3f> kernel{kernelSize};
	for (size_t i = 0; i < kernelSize; i++) {
		// Random vector in z+ hemisphere.
		kernel[i] = vec3f{distr1(gen), distr1(gen), distr2(gen)}.normalize();
		// Scale it so it has length between 0 and 1.
		//kernel[i] *= distr2(gen); // Naive solution
		// More points closer to base, see: http://john-chapman-graphics.blogspot.se/2013/01/ssao-tutorial.html
		float scale = (float)i / (float)kernelSize;
		scale = lerp(0.1f, 1.0f, scale*scale);
		kernel[i] *= scale;
	}

	std::cout << "Generated SSAO sample kernel (size = " << kernelSize << ") with values: \n";
	for (auto& val : kernel) {
		std::cout << val << "\n";
	}
	std::cout << std::endl;

	return std::move(kernel);
}

GLuint generateNoiseTexture(size_t noiseTexWidth) noexcept
{
	static_assert(sizeof(vec3f) == sizeof(float)*3, "vec3f is padded");

	std::random_device rd;
	std::mt19937_64 gen{rd()};
	std::uniform_real_distribution<float> distr{-1.0f, 1.0f};

	size_t numNoiseValues = noiseTexWidth*noiseTexWidth;
	vector<vec3f> noise{numNoiseValues};
	for (size_t i = 0; i < numNoiseValues; i++) {
		noise[i] = vec3f{distr(gen), distr(gen), 0.0f};
	}

	std::cout << "Generated SSAO noise texture (width = " << noiseTexWidth << ") with values: \n";
	for (auto& val : noise) {
		std::cout << val << "\n";
	}
	std::cout << std::endl;

	GLuint noiseTex;
	glGenTextures(1, &noiseTex);
	glBindTexture(GL_TEXTURE_2D, noiseTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, noiseTexWidth, noiseTex, 0, GL_RGB, GL_FLOAT, noise.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTex;
}

} // anonymous namespace

// Occlusion Framebuffer
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

OcclusionFramebuffer::OcclusionFramebuffer(int width, int height) noexcept
:
	mWidth{width},
	mHeight{height}
{
	// Generate framebuffer
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

	// Color texture
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

	// Check that framebuffer is okay
	sfz_assert_release((glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE));

	// Cleanup
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OcclusionFramebuffer::OcclusionFramebuffer(OcclusionFramebuffer&& other) noexcept
{
	glGenFramebuffers(1, &mFBO);
	glGenTextures(1, &mTexture);

	std::swap(mFBO, other.mFBO);
	std::swap(mTexture, other.mTexture);
	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
}

OcclusionFramebuffer& OcclusionFramebuffer::operator= (OcclusionFramebuffer&& other) noexcept
{
	std::swap(mFBO, other.mFBO);
	std::swap(mTexture, other.mTexture);
	std::swap(mWidth, other.mWidth);
	std::swap(mHeight, other.mHeight);
	return *this;
}

OcclusionFramebuffer::~OcclusionFramebuffer() noexcept
{
	glDeleteTextures(1, &mTexture);
	glDeleteFramebuffers(1, &mFBO);
}


// SSAO
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// SSAO: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SSAO::SSAO(int width, int height, size_t numSamples, size_t noiseTexWidth, float radius) noexcept
:
	mWidth{width},
	mHeight{height},
	mSSAOProgram{compileSSAOShaderProgram()},
	mBlurProgram{compileBlurShaderProgram()},
	mOcclusionFBO{mWidth, mHeight},
	mKernelSize{numSamples > MAX_KERNEL_SIZE ? MAX_KERNEL_SIZE : numSamples},
	mKernel{std::move(generateKernel(mKernelSize))},
	mNoiseTexWidth{noiseTexWidth > MAX_NOISE_TEX_WIDTH ? MAX_NOISE_TEX_WIDTH : noiseTexWidth},
	mNoiseTexture{generateNoiseTexture(mNoiseTexWidth)},
	mRadius{radius}
{ }

SSAO::~SSAO() noexcept
{
	glDeleteProgram(mSSAOProgram);
	glDeleteTextures(1, &mNoiseTexture);
}

// SSAO: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void SSAO::apply(GLuint targetFramebuffer,
                 GLuint colorTex, GLuint depthTex, GLuint normalTex, GLuint posTex,
                 const mat4f& projectionMatrix) noexcept
{

	// Render occlusion texture
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mSSAOProgram);
	//glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mOcclusionFBO.mFBO);
	glViewport(0, 0, mOcclusionFBO.mWidth, mOcclusionFBO.mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Texture buffer uniforms

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	gl::setUniform(mSSAOProgram, "uColorTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	gl::setUniform(mSSAOProgram, "uDepthTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTex);
	gl::setUniform(mSSAOProgram, "uNormalTexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, posTex);
	gl::setUniform(mSSAOProgram, "uPositionTexture", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, mNoiseTexture);
	gl::setUniform(mSSAOProgram, "uNoiseTexture", 4);

	// Other uniforms

	gl::setUniform(mSSAOProgram, "uProjectionMatrix", projectionMatrix);

	gl::setUniform(mSSAOProgram, "uKernelSize", static_cast<int>(mKernelSize));
	gl::setUniform(mSSAOProgram, "uKernel", static_cast<vec3f*>(mKernel.data()), mKernelSize);

	gl::setUniform(mSSAOProgram, "uNoiseTexCoordScale",
	     vec2f{(float)mWidth, (float)mHeight} / (float)mNoiseTexWidth);
	gl::setUniform(mSSAOProgram, "uRadius", mRadius);

	mFullscreenQuad.render();

	// Blur occlusion texture
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	glUseProgram(mBlurProgram);
	glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
	glViewport(0, 0, mWidth, mHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mOcclusionFBO.mTexture);
	gl::setUniform(mBlurProgram, "uOcclusionTexture", 0);

	mFullscreenQuad.render();
}

void SSAO::setSize(int width, int height) noexcept
{
	mWidth = width;
	mHeight = height;
	mOcclusionFBO = OcclusionFramebuffer{mWidth, mHeight};
}

} // namespace vox

#include "sfz/MSVC12HackOFF.hpp"