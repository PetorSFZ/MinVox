#include "sfz/gl/SMAA.hpp"

#include <algorithm>

#include "sfz/gl/OpenGL.hpp"

#include "sfz/gl/AreaTex.h"
#include "sfz/gl/SearchTex.h"

namespace gl {

static void flipImage(uint8_t* const pixels, int w, int h, int pitch, int numChannels) noexcept
{
	const int bytesPerRow = w*numChannels;
	const int bytePitch = pitch*numChannels;
	uint8_t* const buffer = new (std::nothrow) uint8_t[bytesPerRow];

	for (int i = 0; i < (h/2); ++i) {
		uint8_t* begin = pixels + i*bytePitch;
		uint8_t* end = pixels + (h-i-1)*bytePitch;

		std::memcpy(buffer, begin, bytesPerRow);
		std::memcpy(begin, end, bytesPerRow);
		std::memcpy(end, buffer, bytesPerRow);
	}

	delete[] buffer;
}

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

static const char SMAA_WEIGHT_CALCULATION_VERTEX_SRC[] = R"(
	#version 410

	// Input
	in vec3 inPosition;
	in vec3 inNormal;
	in vec2 inUV;
	in int inMaterialID;

	// Output
	out vec2 uvCoord;
	out vec2 pixcoord;
	out vec4 offset[3];
	
	// Uniform
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
	 * Blend Weight Calculation Vertex Shader
	 */
	void SMAABlendingWeightCalculationVS(float2 texcoord,
										 out float2 pixcoord,
										 out float4 offset[3]) {
		pixcoord = texcoord * SMAA_RT_METRICS.zw;

		// We will use these offsets for the searches later on (see @PSEUDO_GATHER4):
		offset[0] = mad(SMAA_RT_METRICS.xyxy, float4(-0.25, API_V_DIR(-0.125),  1.25, API_V_DIR(-0.125)), texcoord.xyxy);
		offset[1] = mad(SMAA_RT_METRICS.xyxy, float4(-0.125, API_V_DIR(-0.25), -0.125,  API_V_DIR(1.25)), texcoord.xyxy);

		// And these for the searches, they indicate the ends of the loops:
		offset[2] = mad(SMAA_RT_METRICS.xxyy,
						float4(-2.0, 2.0, API_V_DIR(-2.0), API_V_DIR(2.0)) * float(SMAA_MAX_SEARCH_STEPS),
						float4(offset[0].xz, offset[1].yw));
	}


	void main()
	{
		gl_Position = vec4(inPosition, 1.0);
		uvCoord = inUV;

		SMAABlendingWeightCalculationVS(uvCoord, pixcoord, offset);
	}
)";

static const char SMAA_WEIGHT_CALCULATION_FRAGMENT_SRC[] = R"(
	#version 410

	// Input
	in vec2 uvCoord;
	in vec2 pixcoord;
	in vec4 offset[3];

	// Output
	out vec4 outFragColor;

	// Uniform
	uniform sampler2D uEdgesTexture;
	uniform sampler2D uAreaTexture;
	uniform sampler2D uSearchTexture;
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

	#define SMAA_AREATEX_MAX_DISTANCE 16
	#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
	#define SMAA_AREATEX_PIXEL_SIZE (1.0 / float2(160.0, 560.0))
	#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
	#define SMAA_SEARCHTEX_SIZE float2(66.0, 33.0)
	#define SMAA_SEARCHTEX_PACKED_SIZE float2(64.0, 16.0)
	#define SMAA_CORNER_ROUNDING_NORM (float(SMAA_CORNER_ROUNDING) / 100.0)

	#ifndef SMAA_AREATEX_SELECT
	#if defined(SMAA_HLSL_3)
	#define SMAA_AREATEX_SELECT(sample) sample.ra
	#else
	#define SMAA_AREATEX_SELECT(sample) sample.rg
	#endif
	#endif

	#ifndef SMAA_SEARCHTEX_SELECT
	#define SMAA_SEARCHTEX_SELECT(sample) sample.r
	#endif

	void SMAAMovc(bool2 cond, inout float2 variable, float2 value) {
		SMAA_FLATTEN if (cond.x) variable.x = value.x;
		SMAA_FLATTEN if (cond.y) variable.y = value.y;
	}

	void SMAAMovc(bool4 cond, inout float4 variable, float4 value) {
		SMAAMovc(cond.xy, variable.xy, value.xy);
		SMAAMovc(cond.zw, variable.zw, value.zw);
	}

	//-----------------------------------------------------------------------------
	// Diagonal Search Functions

	#if !defined(SMAA_DISABLE_DIAG_DETECTION)

	float2 SMAADecodeDiagBilinearAccess(float2 e) {
		e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);
		return round(e);
	}

	float4 SMAADecodeDiagBilinearAccess(float4 e) {
		e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);
		return round(e);
	}

	float2 SMAASearchDiag1(SMAATexture2D(edgesTex), float2 texcoord, float2 dir, out float2 e) {
		dir.y = API_V_DIR(dir.y);
		float4 coord = float4(texcoord, -1.0, 1.0);
		float3 t = float3(SMAA_RT_METRICS.xy, 1.0);
		while (coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) &&
			   coord.w > 0.9) {
			coord.xyz = mad(t, float3(dir, 1.0), coord.xyz);
			e = SMAASampleLevelZero(edgesTex, coord.xy).rg;
			coord.w = dot(e, float2(0.5, 0.5));
		}
		return coord.zw;
	}

	float2 SMAASearchDiag2(SMAATexture2D(edgesTex), float2 texcoord, float2 dir, out float2 e) {
		dir.y = API_V_DIR(dir.y);
		float4 coord = float4(texcoord, -1.0, 1.0);
		coord.x += 0.25 * SMAA_RT_METRICS.x; // See @SearchDiag2Optimization
		float3 t = float3(SMAA_RT_METRICS.xy, 1.0);
		while (coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) &&
			   coord.w > 0.9) {
			coord.xyz = mad(t, float3(dir, 1.0), coord.xyz);
			e = SMAASampleLevelZero(edgesTex, coord.xy).rg;
			e = SMAADecodeDiagBilinearAccess(e);
			coord.w = dot(e, float2(0.5, 0.5));
		}
		return coord.zw;
	}

	float2 SMAAAreaDiag(SMAATexture2D(areaTex), float2 dist, float2 e, float offset) {
		float2 texcoord = mad(float2(SMAA_AREATEX_MAX_DISTANCE_DIAG, SMAA_AREATEX_MAX_DISTANCE_DIAG), e, dist);
		texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);
		texcoord.x += 0.5;
		texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;
		texcoord.y = API_V_COORD(texcoord.y);
		return SMAA_AREATEX_SELECT(SMAASampleLevelZero(areaTex, texcoord));
	}

	float2 SMAACalculateDiagWeights(SMAATexture2D(edgesTex), SMAATexture2D(areaTex), float2 texcoord, float2 e, float4 subsampleIndices) {
		float2 weights = float2(0.0, 0.0);
		float4 d;
		float2 end;
		if (e.r > 0.0) {
			d.xz = SMAASearchDiag1(SMAATexturePass2D(edgesTex), texcoord, float2(-1.0,  1.0), end);
			d.x += float(end.y > 0.9);
		} else
			d.xz = float2(0.0, 0.0);
		d.yw = SMAASearchDiag1(SMAATexturePass2D(edgesTex), texcoord, float2(1.0, -1.0), end);

		SMAA_BRANCH
		if (d.x + d.y > 2.0) { // d.x + d.y + 1 > 3
			// Fetch the crossing edges:
			float4 coords = mad(float4(-d.x + 0.25, API_V_DIR(d.x), d.y, API_V_DIR(-d.y - 0.25)), SMAA_RT_METRICS.xyxy, texcoord.xyxy);
			float4 c;
			c.xy = SMAASampleLevelZeroOffset(edgesTex, coords.xy, int2(-1,  0)).rg;
			c.zw = SMAASampleLevelZeroOffset(edgesTex, coords.zw, int2( 1,  0)).rg;
			c.yxwz = SMAADecodeDiagBilinearAccess(c.xyzw);
			float2 cc = mad(float2(2.0, 2.0), c.xz, c.yw);
			SMAAMovc(bool2(step(0.9, d.zw)), cc, float2(0.0, 0.0));
			weights += SMAAAreaDiag(SMAATexturePass2D(areaTex), d.xy, cc, subsampleIndices.z);
		}

		d.xz = SMAASearchDiag2(SMAATexturePass2D(edgesTex), texcoord, float2(-1.0, -1.0), end);
		if (SMAASampleLevelZeroOffset(edgesTex, texcoord, int2(1, 0)).r > 0.0) {
			d.yw = SMAASearchDiag2(SMAATexturePass2D(edgesTex), texcoord, float2(1.0, 1.0), end);
			d.y += float(end.y > 0.9);
		} else
			d.yw = float2(0.0, 0.0);

		SMAA_BRANCH
		if (d.x + d.y > 2.0) { // d.x + d.y + 1 > 3
			float4 coords = mad(float4(-d.x, API_V_DIR(-d.x), d.y, API_V_DIR(d.y)), SMAA_RT_METRICS.xyxy, texcoord.xyxy);
			float4 c;
			c.x  = SMAASampleLevelZeroOffset(edgesTex, coords.xy, int2(-1,  0)).g;
			c.y  = SMAASampleLevelZeroOffset(edgesTex, coords.xy, int2( 0, API_V_DIR(-1))).r;
			c.zw = SMAASampleLevelZeroOffset(edgesTex, coords.zw, int2( 1,  0)).gr;
			float2 cc = mad(float2(2.0, 2.0), c.xz, c.yw);
			SMAAMovc(bool2(step(0.9, d.zw)), cc, float2(0.0, 0.0));
			weights += SMAAAreaDiag(SMAATexturePass2D(areaTex), d.xy, cc, subsampleIndices.w).gr;
		}

		return weights;
	}
	#endif

	//-----------------------------------------------------------------------------
	// Horizontal/Vertical Search Functions

	float SMAASearchLength(SMAATexture2D(searchTex), float2 e, float offset) {
		float2 scale = SMAA_SEARCHTEX_SIZE * float2(0.5, -1.0);
		float2 bias = SMAA_SEARCHTEX_SIZE * float2(offset, 1.0);
		scale += float2(-1.0,  1.0);
		bias  += float2( 0.5, -0.5);
		scale *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
		bias *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
		float2 coord = mad(scale, e, bias);
		coord.y = API_V_COORD(coord.y);
		return SMAA_SEARCHTEX_SELECT(SMAASampleLevelZero(searchTex, coord));
	}

	float SMAASearchXLeft(SMAATexture2D(edgesTex), SMAATexture2D(searchTex), float2 texcoord, float end) {
		float2 e = float2(0.0, 1.0);
		while (texcoord.x > end && 
			   e.g > 0.8281 && // Is there some edge not activated?
			   e.r == 0.0) { // Or is there a crossing edge that breaks the line?
			e = SMAASampleLevelZero(edgesTex, texcoord).rg;
			texcoord = mad(-float2(2.0, 0.0), SMAA_RT_METRICS.xy, texcoord);
		}

		float offset = mad(-(255.0 / 127.0), SMAASearchLength(SMAATexturePass2D(searchTex), e, 0.0), 3.25);
		return mad(SMAA_RT_METRICS.x, offset, texcoord.x);
	}

	float SMAASearchXRight(SMAATexture2D(edgesTex), SMAATexture2D(searchTex), float2 texcoord, float end) {
		float2 e = float2(0.0, 1.0);
		while (texcoord.x < end && 
			   e.g > 0.8281 && // Is there some edge not activated?
			   e.r == 0.0) { // Or is there a crossing edge that breaks the line?
			e = SMAASampleLevelZero(edgesTex, texcoord).rg;
			texcoord = mad(float2(2.0, 0.0), SMAA_RT_METRICS.xy, texcoord);
		}
		float offset = mad(-(255.0 / 127.0), SMAASearchLength(SMAATexturePass2D(searchTex), e, 0.5), 3.25);
		return mad(-SMAA_RT_METRICS.x, offset, texcoord.x);
	}

	float SMAASearchYUp(SMAATexture2D(edgesTex), SMAATexture2D(searchTex), float2 texcoord, float end) {
		float2 e = float2(1.0, 0.0);
		while (API_V_BELOW(texcoord.y, end) && 
			   e.r > 0.8281 && // Is there some edge not activated?
			   e.g == 0.0) { // Or is there a crossing edge that breaks the line?
			e = SMAASampleLevelZero(edgesTex, texcoord).rg;
			texcoord = mad(-float2(0.0, API_V_DIR(2.0)), SMAA_RT_METRICS.xy, texcoord);
		}
		float offset = mad(-(255.0 / 127.0), SMAASearchLength(SMAATexturePass2D(searchTex), e.gr, 0.0), 3.25);
		return mad(SMAA_RT_METRICS.y, API_V_DIR(offset), texcoord.y);
	}

	float SMAASearchYDown(SMAATexture2D(edgesTex), SMAATexture2D(searchTex), float2 texcoord, float end) {
		float2 e = float2(1.0, 0.0);
		while (API_V_ABOVE(texcoord.y, end) && 
			   e.r > 0.8281 && // Is there some edge not activated?
			   e.g == 0.0) { // Or is there a crossing edge that breaks the line?
			e = SMAASampleLevelZero(edgesTex, texcoord).rg;
			texcoord = mad(float2(0.0, API_V_DIR(2.0)), SMAA_RT_METRICS.xy, texcoord);
		}
		float offset = mad(-(255.0 / 127.0), SMAASearchLength(SMAATexturePass2D(searchTex), e.gr, 0.5), 3.25);
		return mad(-SMAA_RT_METRICS.y, API_V_DIR(offset), texcoord.y);
	}

	float2 SMAAArea(SMAATexture2D(areaTex), float2 dist, float e1, float e2, float offset) {
		float2 texcoord = mad(float2(SMAA_AREATEX_MAX_DISTANCE, SMAA_AREATEX_MAX_DISTANCE), round(4.0 * float2(e1, e2)), dist);
		texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);
		texcoord.y = mad(SMAA_AREATEX_SUBTEX_SIZE, offset, texcoord.y);
		texcoord.y = API_V_COORD(texcoord.y);
		return SMAA_AREATEX_SELECT(SMAASampleLevelZero(areaTex, texcoord));
	}

	//-----------------------------------------------------------------------------
	// Corner Detection Functions

	void SMAADetectHorizontalCornerPattern(SMAATexture2D(edgesTex), inout float2 weights, float4 texcoord, float2 d) {
		#if !defined(SMAA_DISABLE_CORNER_DETECTION)
		float2 leftRight = step(d.xy, d.yx);
		float2 rounding = (1.0 - SMAA_CORNER_ROUNDING_NORM) * leftRight;

		rounding /= leftRight.x + leftRight.y; // Reduce blending for pixels in the center of a line.

		float2 factor = float2(1.0, 1.0);
		factor.x -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, int2(0,  API_V_DIR(1))).r;
		factor.x -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, int2(1,  API_V_DIR(1))).r;
		factor.y -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, int2(0, API_V_DIR(-2))).r;
		factor.y -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, int2(1, API_V_DIR(-2))).r;

		weights *= saturate(factor);
		#endif
	}

	void SMAADetectVerticalCornerPattern(SMAATexture2D(edgesTex), inout float2 weights, float4 texcoord, float2 d) {
		#if !defined(SMAA_DISABLE_CORNER_DETECTION)
		float2 leftRight = step(d.xy, d.yx);
		float2 rounding = (1.0 - SMAA_CORNER_ROUNDING_NORM) * leftRight;

		rounding /= leftRight.x + leftRight.y;

		float2 factor = float2(1.0, 1.0);
		factor.x -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, int2( 1, 0)).g;
		factor.x -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, int2( 1, API_V_DIR(1))).g;
		factor.y -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, int2(-2, 0)).g;
		factor.y -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, int2(-2, API_V_DIR(1))).g;

		weights *= saturate(factor);
		#endif
	}

	float4 SMAABlendingWeightCalculationPS(float2 texcoord,
										   float2 pixcoord,
										   float4 offset[3],
										   SMAATexture2D(edgesTex),
										   SMAATexture2D(areaTex),
										   SMAATexture2D(searchTex),
										   float4 subsampleIndices) { // Just pass zero for SMAA 1x, see @SUBSAMPLE_INDICES.
		float4 weights = float4(0.0, 0.0, 0.0, 0.0);

		float2 e = SMAASample(edgesTex, texcoord).rg;

		SMAA_BRANCH
		if (e.g > 0.0) { // Edge at north
			#if !defined(SMAA_DISABLE_DIAG_DETECTION)
			// Diagonals have both north and west edges, so searching for them in
			// one of the boundaries is enough.
			weights.rg = SMAACalculateDiagWeights(SMAATexturePass2D(edgesTex), SMAATexturePass2D(areaTex), texcoord, e, subsampleIndices);

			// We give priority to diagonals, so if we find a diagonal we skip 
			// horizontal/vertical processing.
			SMAA_BRANCH
			if (weights.r == -weights.g) { // weights.r + weights.g == 0.0
			#endif

			float2 d;

			// Find the distance to the left:
			float3 coords;
			coords.x = SMAASearchXLeft(SMAATexturePass2D(edgesTex), SMAATexturePass2D(searchTex), offset[0].xy, offset[2].x);
			coords.y = offset[1].y; // offset[1].y = texcoord.y - 0.25 * SMAA_RT_METRICS.y (@CROSSING_OFFSET)
			d.x = coords.x;

			// Now fetch the left crossing edges, two at a time using bilinear
			// filtering. Sampling at -0.25 (see @CROSSING_OFFSET) enables to
			// discern what value each edge has:
			float e1 = SMAASampleLevelZero(edgesTex, coords.xy).r;

			// Find the distance to the right:
			coords.z = SMAASearchXRight(SMAATexturePass2D(edgesTex), SMAATexturePass2D(searchTex), offset[0].zw, offset[2].y);
			d.y = coords.z;

			// We want the distances to be in pixel units (doing this here allow to
			// better interleave arithmetic and memory accesses):
			d = abs(round(mad(SMAA_RT_METRICS.zz, d, -pixcoord.xx)));

			// SMAAArea below needs a sqrt, as the areas texture is compressed
			// quadratically:
			float2 sqrt_d = sqrt(d);

			// Fetch the right crossing edges:
			float e2 = SMAASampleLevelZeroOffset(edgesTex, coords.zy, int2(1, 0)).r;

			// Ok, we know how this pattern looks like, now it is time for getting
			// the actual area:
			weights.rg = SMAAArea(SMAATexturePass2D(areaTex), sqrt_d, e1, e2, subsampleIndices.y);

			// Fix corners:
			coords.y = texcoord.y;
			SMAADetectHorizontalCornerPattern(SMAATexturePass2D(edgesTex), weights.rg, coords.xyzy, d);

			#if !defined(SMAA_DISABLE_DIAG_DETECTION)
			} else
				e.r = 0.0; // Skip vertical processing.
			#endif
		}

		SMAA_BRANCH
		if (e.r > 0.0) { // Edge at west
			float2 d;

			// Find the distance to the top:
			float3 coords;
			coords.y = SMAASearchYUp(SMAATexturePass2D(edgesTex), SMAATexturePass2D(searchTex), offset[1].xy, offset[2].z);
			coords.x = offset[0].x; // offset[1].x = texcoord.x - 0.25 * SMAA_RT_METRICS.x;
			d.x = coords.y;

			// Fetch the top crossing edges:
			float e1 = SMAASampleLevelZero(edgesTex, coords.xy).g;

			// Find the distance to the bottom:
			coords.z = SMAASearchYDown(SMAATexturePass2D(edgesTex), SMAATexturePass2D(searchTex), offset[1].zw, offset[2].w);
			d.y = coords.z;

			// We want the distances to be in pixel units:
			d = abs(round(mad(SMAA_RT_METRICS.ww, d, -pixcoord.yy)));

			// SMAAArea below needs a sqrt, as the areas texture is compressed 
			// quadratically:
			float2 sqrt_d = sqrt(d);

			// Fetch the bottom crossing edges:
			float e2 = SMAASampleLevelZeroOffset(edgesTex, coords.xz, int2(0, API_V_DIR(1))).g;

			// Get the area for this direction:
			weights.ba = SMAAArea(SMAATexturePass2D(areaTex), sqrt_d, e1, e2, subsampleIndices.x);

			// Fix corners:
			coords.x = texcoord.x;
			SMAADetectVerticalCornerPattern(SMAATexturePass2D(edgesTex), weights.ba, coords.xyxz, d);
		}

		return weights;
	}


	void main()
	{
		vec4 res = SMAABlendingWeightCalculationPS(uvCoord, pixcoord, offset, uEdgesTexture, uAreaTexture, uSearchTexture, vec4(0.0));
		outFragColor = res;
	}
)";

static const char SMAA_BLENDING_VERTEX_SRC[] = R"(
	#version 410
	
	// Input
	in vec3 inPosition;
	in vec3 inNormal;
	in vec2 inUV;
	in int inMaterialID;

	// Output
	out vec2 uvCoord;
	out vec4 offset;
	
	// Uniform
	uniform vec2 uPixelDim;

	#define SMAA_GLSL_4
	vec4 SMAA_RT_METRICS = vec4(uPixelDim, 1.0 / uPixelDim);
	#define SMAA_PRESET_ULTRA

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
	 * Neighborhood Blending Vertex Shader
	 */
	void SMAANeighborhoodBlendingVS(float2 texcoord,
									out float4 offset) {
		offset = mad(SMAA_RT_METRICS.xyxy, float4( 1.0, 0.0, 0.0, API_V_DIR(1.0)), texcoord.xyxy);
	}

	void main()
	{
		gl_Position = vec4(inPosition, 1.0);
		uvCoord = inUV;

		SMAANeighborhoodBlendingVS(uvCoord, offset);
	}
)";

static const char SMAA_BLENDING_FRAGMENT_SRC[] = R"(
	#version 410

	// Input
	in vec2 uvCoord;
	in vec4 offset;

	// Output
	out vec4 outFragColor;

	// Uniforms
	uniform vec2 uPixelDim;
	uniform sampler2D uInputTexture;
	uniform sampler2D uWeightsTexture;

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

	void SMAAMovc(bool2 cond, inout float2 variable, float2 value) {
		SMAA_FLATTEN if (cond.x) variable.x = value.x;
		SMAA_FLATTEN if (cond.y) variable.y = value.y;
	}

	void SMAAMovc(bool4 cond, inout float4 variable, float4 value) {
		SMAAMovc(cond.xy, variable.xy, value.xy);
		SMAAMovc(cond.zw, variable.zw, value.zw);
	}

	//-----------------------------------------------------------------------------
	// Neighborhood Blending Pixel Shader (Third Pass)

	float4 SMAANeighborhoodBlendingPS(float2 texcoord,
									  float4 offset,
									  SMAATexture2D(colorTex),
									  SMAATexture2D(blendTex)
									  #if SMAA_REPROJECTION
									  , SMAATexture2D(velocityTex)
									  #endif
									  ) {
		// Fetch the blending weights for current pixel:
		float4 a;
		a.x = SMAASample(blendTex, offset.xy).a; // Right
		a.y = SMAASample(blendTex, offset.zw).g; // Top
		a.wz = SMAASample(blendTex, texcoord).xz; // Bottom / Left

		// Is there any blending weight with a value greater than 0.0?
		SMAA_BRANCH
		if (dot(a, float4(1.0, 1.0, 1.0, 1.0)) < 1e-5) {
			float4 color = SMAASampleLevelZero(colorTex, texcoord);

			#if SMAA_REPROJECTION
			float2 velocity = SMAA_DECODE_VELOCITY(SMAASampleLevelZero(velocityTex, texcoord));

			// Pack velocity into the alpha channel:
			color.a = sqrt(5.0 * length(velocity));
			#endif

			return color;
		} else {
			bool h = max(a.x, a.z) > max(a.y, a.w); // max(horizontal) > max(vertical)

			// Calculate the blending offsets:
			float4 blendingOffset = float4(0.0, API_V_DIR(a.y), 0.0, API_V_DIR(a.w));
			float2 blendingWeight = a.yw;
			SMAAMovc(bool4(h, h, h, h), blendingOffset, float4(a.x, 0.0, a.z, 0.0));
			SMAAMovc(bool2(h, h), blendingWeight, a.xz);
			blendingWeight /= dot(blendingWeight, float2(1.0, 1.0));

			// Calculate the texture coordinates:
			float4 blendingCoord = mad(blendingOffset, float4(SMAA_RT_METRICS.xy, -SMAA_RT_METRICS.xy), texcoord.xyxy);

			// We exploit bilinear filtering to mix current pixel with the chosen
			// neighbor:
			float4 color = blendingWeight.x * SMAASampleLevelZero(colorTex, blendingCoord.xy);
			color += blendingWeight.y * SMAASampleLevelZero(colorTex, blendingCoord.zw);

			#if SMAA_REPROJECTION
			// Antialias velocity for proper reprojection in a later stage:
			float2 velocity = blendingWeight.x * SMAA_DECODE_VELOCITY(SMAASampleLevelZero(velocityTex, blendingCoord.xy));
			velocity += blendingWeight.y * SMAA_DECODE_VELOCITY(SMAASampleLevelZero(velocityTex, blendingCoord.zw));

			// Pack velocity into the alpha channel:
			color.a = sqrt(5.0 * length(velocity));
			#endif

			return color;
		}
	}

	void main()
	{
		vec4 res = SMAANeighborhoodBlendingPS(uvCoord, offset, uInputTexture, uWeightsTexture);
		outFragColor = res;
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
	mSMAAWeightCalculation = Program::fromSource(SMAA_WEIGHT_CALCULATION_VERTEX_SRC,
	                                             SMAA_WEIGHT_CALCULATION_FRAGMENT_SRC,
	                                             [](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindAttribLocation(shaderProgram, 1, "inNormal");
		glBindAttribLocation(shaderProgram, 2, "inUV");
		glBindAttribLocation(shaderProgram, 3, "inMaterialID");
	});
	mSMAABlending = Program::fromSource(SMAA_BLENDING_VERTEX_SRC,
	                                    SMAA_BLENDING_FRAGMENT_SRC,
	                                    [](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindAttribLocation(shaderProgram, 1, "inNormal");
		glBindAttribLocation(shaderProgram, 2, "inUV");
		glBindAttribLocation(shaderProgram, 3, "inMaterialID");
	});

	mEdgesFB = FramebufferBuilder(dimensions)
	          .addTexture(0, FBTextureFormat::RG_U8, FBTextureFiltering::LINEAR)
	          .build();
	mWeightsFB = FramebufferBuilder(dimensions)
	            .addTexture(0, FBTextureFormat::RGBA_U8, FBTextureFiltering::LINEAR)
	            .build();
	mResultFB  = FramebufferBuilder(dimensions)
	            .addTexture(0, FBTextureFormat::RGB_U8, FBTextureFiltering::LINEAR)
	            .build();


	unsigned char* areaTexBytesCopy = new unsigned char[AREATEX_SIZE];
	std::memcpy(areaTexBytesCopy, areaTexBytes, AREATEX_SIZE);
	flipImage(areaTexBytesCopy, AREATEX_WIDTH, AREATEX_HEIGHT, AREATEX_WIDTH, 2);

	glGenTextures(1, &mAreaTex);
	glBindTexture(GL_TEXTURE_2D, mAreaTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, AREATEX_WIDTH, AREATEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, areaTexBytesCopy);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	unsigned char* searchTexBytesCopy = new unsigned char[SEARCHTEX_SIZE];
	std::memcpy(searchTexBytesCopy, searchTexBytes, SEARCHTEX_SIZE);
	flipImage(searchTexBytesCopy, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, SEARCHTEX_WIDTH, 1);

	glGenTextures(1, &mSearchTex);
	glBindTexture(GL_TEXTURE_2D, mSearchTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, GL_RG, GL_UNSIGNED_BYTE, searchTexBytesCopy);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	delete[] areaTexBytesCopy;
	delete[] searchTexBytesCopy;
}

SMAA::SMAA(SMAA&& other) noexcept
{
	*this = std::move(other);
}

SMAA& SMAA::operator= (SMAA&& other)
{
	std::swap(this->mDimensions, other.mDimensions);
	this->mPostProcessQuad = std::move(other.mPostProcessQuad);
	this->mSMAAEdgeDetection = std::move(other.mSMAAEdgeDetection);
	this->mSMAAWeightCalculation = std::move(other.mSMAAWeightCalculation);
	this->mSMAABlending = std::move(other.mSMAABlending);
	this->mEdgesFB = std::move(other.mEdgesFB);
	this->mWeightsFB = std::move(other.mWeightsFB);
	this->mResultFB = std::move(other.mResultFB);

	std::swap(this->mAreaTex, other.mAreaTex);
	std::swap(this->mSearchTex, other.mSearchTex);

	return *this;
}

SMAA::~SMAA() noexcept
{
	glDeleteTextures(1, &mAreaTex);
	glDeleteTextures(1, &mSearchTex);
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


	// Weight calculation
	glUseProgram(mSMAAWeightCalculation.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mWeightsFB.fbo());
	glViewport(0, 0, mWeightsFB.width(), mWeightsFB.height());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	gl::setUniform(mSMAAWeightCalculation, "uPixelDim", vec2(1.0) / mEdgesFB.dimensionsFloat());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mEdgesFB.texture(0));
	gl::setUniform(mSMAAWeightCalculation, "uEdgesTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mAreaTex);
	gl::setUniform(mSMAAWeightCalculation, "uAreaTexture", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mSearchTex);
	gl::setUniform(mSMAAWeightCalculation, "uSearchTexture", 2);

	mPostProcessQuad.render();


	// Neighborhood blending
	glUseProgram(mSMAABlending.handle());
	glBindFramebuffer(GL_FRAMEBUFFER, mResultFB.fbo());
	glViewport(0, 0, mResultFB.width(), mResultFB.height());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	gl::setUniform(mSMAABlending, "uPixelDim", vec2(1.0) / mResultFB.dimensionsFloat());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	gl::setUniform(mSMAABlending, "uInputTexture", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mWeightsFB.texture(0));
	gl::setUniform(mSMAABlending, "uWeightsTexture", 1);

	mPostProcessQuad.render();


	return mResultFB.texture(0);
}

} // namespace gl
