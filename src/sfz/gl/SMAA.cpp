#include "sfz/gl/SMAA.hpp"

#include "sfz/gl/OpenGL.hpp"

#include "sfz/gl/AreaTex.h"
#include "sfz/gl/SearchTex.h"

namespace gl {

static const char SMAA_EDGE_DETECTION_VERTEX_SRC[] = R"(
	#version 410

	// Input
	in vec3 inPosition;
	in vec3 inNormal;
	in vec2 inUV;
	in int inMaterialID;

	// Output
	out vec2 uvCoord;
	out vec4 offset[3];
	
	// Uniform
	uniform vec2 uPixelDim;

	#define SMAA_GLSL_4
	vec4 SMAA_RT_METRICS = vec4(uPixelDim, 1.0 / uPixelDim);


	#if defined(SMAA_GLSL_3) || defined(SMAA_GLSL_4)
	#define API_V_DIR(v) -(v)
	#define API_V_COORD(v) (1.0 - v)
	#define API_V_BELOW(v1, v2)	v1 < v2
	#define API_V_ABOVE(v1, v2)	v1 > v2
	#define SMAATexture2D(tex) sampler2D tex
	#define SMAATexturePass2D(tex) tex
	#define SMAASampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
	#define SMAASampleLevelZeroPoint(tex, coord) textureLod(tex, coord, 0.0)
	#define SMAASampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
	#define SMAASample(tex, coord) texture2D(tex, coord)
	#define SMAASamplePoint(tex, coord) texture2D(tex, coord)
	#define SMAASampleOffset(tex, coord, offset) texture(tex, coord, offset)
	#define SMAA_FLATTEN
	#define SMAA_BRANCH
	#define lerp(a, b, t) mix(a, b, t)
	#define saturate(a) clamp(a, 0.0, 1.0)
	#if defined(SMAA_GLSL_4)
	#define mad(a, b, c) fma(a, b, c)
	#define SMAAGather(tex, coord) textureGather(tex, coord)
	#else
	#define mad(a, b, c) (a * b + c)
	#endif
	#define float2 vec2
	#define float3 vec3
	#define float4 vec4
	#define int2 ivec2
	#define int3 ivec3
	#define int4 ivec4
	#define bool2 bvec2
	#define bool3 bvec3
	#define bool4 bvec4
	#endif

	/**
	 * Edge Detection Vertex Shader
	 */
	void SMAAEdgeDetectionVS(float2 texcoord,
							 out float4 offset[3]) {
		offset[0] = mad(SMAA_RT_METRICS.xyxy, float4(-1.0, 0.0, 0.0, API_V_DIR(-1.0)), texcoord.xyxy);
		offset[1] = mad(SMAA_RT_METRICS.xyxy, float4( 1.0, 0.0, 0.0, API_V_DIR(1.0)), texcoord.xyxy);
		offset[2] = mad(SMAA_RT_METRICS.xyxy, float4(-2.0, 0.0, 0.0, API_V_DIR(-2.0)), texcoord.xyxy);
	}

	void main()
	{
		gl_Position = vec4(inPosition, 1.0);
		uvCoord = inUV;

		SMAAEdgeDetectionVS(uvCoord, offset);
	}
)";

static const char SMAA_EDGE_DETECTION_FRAGMENT_SRC[] = R"(
	#version 410

	// Input
	in vec2 uvCoord;
	in vec4 offset[3];

	// Output
	out vec4 outFragColor;

	// Uniform
	uniform sampler2D uInputTexture;
	uniform vec2 uPixelDim;

	#define SMAA_GLSL_4
	vec4 SMAA_RT_METRICS = vec4(uPixelDim, 1.0 / uPixelDim);
	#define SMAA_PRESET_ULTRA

	#if defined(SMAA_PRESET_LOW)
	#define SMAA_THRESHOLD 0.15
	#define SMAA_MAX_SEARCH_STEPS 4
	#define SMAA_DISABLE_DIAG_DETECTION
	#define SMAA_DISABLE_CORNER_DETECTION
	#elif defined(SMAA_PRESET_MEDIUM)
	#define SMAA_THRESHOLD 0.1
	#define SMAA_MAX_SEARCH_STEPS 8
	#define SMAA_DISABLE_DIAG_DETECTION
	#define SMAA_DISABLE_CORNER_DETECTION
	#elif defined(SMAA_PRESET_HIGH)
	#define SMAA_THRESHOLD 0.1
	#define SMAA_MAX_SEARCH_STEPS 16
	#define SMAA_MAX_SEARCH_STEPS_DIAG 8
	#define SMAA_CORNER_ROUNDING 25
	#elif defined(SMAA_PRESET_ULTRA)
	#define SMAA_THRESHOLD 0.05
	#define SMAA_MAX_SEARCH_STEPS 32
	#define SMAA_MAX_SEARCH_STEPS_DIAG 16
	#define SMAA_CORNER_ROUNDING 25
	#endif

	/**
	 * If there is an neighbor edge that has SMAA_LOCAL_CONTRAST_FACTOR times
	 * bigger contrast than current edge, current edge will be discarded.
	 *
	 * This allows to eliminate spurious crossing edges, and is based on the fact
	 * that, if there is too much contrast in a direction, that will hide
	 * perceptually contrast in the other neighbors.
	 */
	#ifndef SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR
	#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0
	#endif

	#if defined(SMAA_GLSL_3) || defined(SMAA_GLSL_4)
	#define API_V_DIR(v) -(v)
	#define API_V_COORD(v) (1.0 - v)
	#define API_V_BELOW(v1, v2)	v1 < v2
	#define API_V_ABOVE(v1, v2)	v1 > v2
	#define SMAATexture2D(tex) sampler2D tex
	#define SMAATexturePass2D(tex) tex
	#define SMAASampleLevelZero(tex, coord) textureLod(tex, coord, 0.0)
	#define SMAASampleLevelZeroPoint(tex, coord) textureLod(tex, coord, 0.0)
	#define SMAASampleLevelZeroOffset(tex, coord, offset) textureLodOffset(tex, coord, 0.0, offset)
	#define SMAASample(tex, coord) texture2D(tex, coord)
	#define SMAASamplePoint(tex, coord) texture2D(tex, coord)
	#define SMAASampleOffset(tex, coord, offset) texture(tex, coord, offset)
	#define SMAA_FLATTEN
	#define SMAA_BRANCH
	#define lerp(a, b, t) mix(a, b, t)
	#define saturate(a) clamp(a, 0.0, 1.0)
	#if defined(SMAA_GLSL_4)
	#define mad(a, b, c) fma(a, b, c)
	#define SMAAGather(tex, coord) textureGather(tex, coord)
	#else
	#define mad(a, b, c) (a * b + c)
	#endif
	#define float2 vec2
	#define float3 vec3
	#define float4 vec4
	#define int2 ivec2
	#define int3 ivec3
	#define int4 ivec4
	#define bool2 bvec2
	#define bool3 bvec3
	#define bool4 bvec4
	#endif

	/**
	 * Luma Edge Detection
	 *
	 * IMPORTANT NOTICE: luma edge detection requires gamma-corrected colors, and
	 * thus 'colorTex' should be a non-sRGB texture.
	 */
	float2 SMAALumaEdgeDetectionPS(float2 texcoord,
								   float4 offset[3],
								   SMAATexture2D(colorTex)
								   #if SMAA_PREDICATION
								   , SMAATexture2D(predicationTex)
								   #endif
								   ) {
		// Calculate the threshold:
		#if SMAA_PREDICATION
		float2 threshold = SMAACalculatePredicatedThreshold(texcoord, offset, SMAATexturePass2D(predicationTex));
		#else
		float2 threshold = float2(SMAA_THRESHOLD, SMAA_THRESHOLD);
		#endif

		// Calculate lumas:
		float3 weights = float3(0.2126, 0.7152, 0.0722);
		float L = dot(SMAASamplePoint(colorTex, texcoord).rgb, weights);

		float Lleft = dot(SMAASamplePoint(colorTex, offset[0].xy).rgb, weights);
		float Ltop  = dot(SMAASamplePoint(colorTex, offset[0].zw).rgb, weights);

		// We do the usual threshold:
		float4 delta;
		delta.xy = abs(L - float2(Lleft, Ltop));
		float2 edges = step(threshold, delta.xy);

		// Then discard if there is no edge:
		if (dot(edges, float2(1.0, 1.0)) == 0.0)
			discard;

		// Calculate right and bottom deltas:
		float Lright = dot(SMAASamplePoint(colorTex, offset[1].xy).rgb, weights);
		float Lbottom  = dot(SMAASamplePoint(colorTex, offset[1].zw).rgb, weights);
		delta.zw = abs(L - float2(Lright, Lbottom));

		// Calculate the maximum delta in the direct neighborhood:
		float2 maxDelta = max(delta.xy, delta.zw);

		// Calculate left-left and top-top deltas:
		float Lleftleft = dot(SMAASamplePoint(colorTex, offset[2].xy).rgb, weights);
		float Ltoptop = dot(SMAASamplePoint(colorTex, offset[2].zw).rgb, weights);
		delta.zw = abs(float2(Lleft, Ltop) - float2(Lleftleft, Ltoptop));

		// Calculate the final maximum delta:
		maxDelta = max(maxDelta.xy, delta.zw);
		float finalDelta = max(maxDelta.x, maxDelta.y);

		// Local contrast adaptation:
		edges.xy *= step(finalDelta, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR * delta.xy);

		return edges;
	}


	void main()
	{
		vec2 res = SMAALumaEdgeDetectionPS(uvCoord, offset, uInputTexture);
		outFragColor = vec4(res, 0.0, 1.0);
	}
)";

// SMAA: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

SMAA::SMAA(vec2i dimensions) noexcept
{
	mDimensions = dimensions;

	mSMAAEdgeDetection = Program::fromSource(SMAA_EDGE_DETECTION_VERTEX_SRC,
	                                         SMAA_EDGE_DETECTION_FRAGMENT_SRC,
	                                         [](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindAttribLocation(shaderProgram, 1, "inNormal");
		glBindAttribLocation(shaderProgram, 2, "inUV");
		glBindAttribLocation(shaderProgram, 3, "inMaterialID");
	});
	mSMAAWeightCalculation;
	mSMAABlending;

	mEdgesFB = FramebufferBuilder(dimensions)
	          .addTexture(0, FBTextureFormat::RG_U8, FBTextureFiltering::LINEAR)
	          .build();
	mWeightsFB = FramebufferBuilder(dimensions)
	            .addTexture(0, FBTextureFormat::R_U8, FBTextureFiltering::LINEAR)
	            .build();
	mResultFB  = FramebufferBuilder(dimensions)
	            .addTexture(0, FBTextureFormat::RGB_U8, FBTextureFiltering::LINEAR)
	            .build();
}

SMAA::~SMAA() noexcept
{
	
}

// SMAA: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

uint32_t SMAA::apply(uint32_t tex) noexcept
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	// Edge detection
	glUseProgram(mSMAAEdgeDetection.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mEdgesFB.fbo());
	glViewport(0, 0, mEdgesFB.width(), mEdgesFB.height());
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	gl::setUniform(mSMAAEdgeDetection, "uPixelDim", vec2(1.0) / mEdgesFB.dimensionsFloat());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	gl::setUniform(mSMAAEdgeDetection, "uInputTexture", 0);

	mPostProcessQuad.render();

	return mEdgesFB.texture(0);
}

} // namespace gl
