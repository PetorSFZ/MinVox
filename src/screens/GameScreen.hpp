#pragma once
#ifndef VOX_SCREENS_BASE_GAME_SCREEN_HPP
#define VOX_SCREENS_BASE_GAME_SCREEN_HPP

#include <string>
#include <memory>
#include <random>
#include <vector>

#include <sfz/GL.hpp>
#include <sfz/Screens.hpp>

#include "IO.hpp"
#include "Model.hpp"
#include "Rendering.hpp"
#include "GlobalConfig.hpp"

namespace vox {

using sfz::vec3;
using sfz::vec4;
using sfz::mat4;

using sfz::ScreenUpdateOp;
using std::unordered_map;
using std::vector;

class GameScreen final : public sfz::BaseScreen {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GameScreen() = delete;
	//GameScreen(const GameScreen&) = delete;
	GameScreen& operator= (const GameScreen&) = delete;

	GameScreen(sdl::Window& window, const std::string& worldName);
	//GameScreen(const GameScreen& baseGameScreen);
	//virtual ~GameScreen();

	// Overriden methods from sfz::BaseScreen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	virtual ScreenUpdateOp update(const vector<SDL_Event>& events,
	                              const unordered_map<int32_t, sdl::GameController>& controllers,
	                              float delta) override final;
	virtual void render(float delta) override final;
	virtual void onQuit() override final;
	virtual void onResize(vec2 dimensions) override final;

private:
	// Private methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void updateResolutions(int width, int height) noexcept;
	void reloadFramebuffers(int width, int height) noexcept;

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GlobalConfig& mCfg;

	World mWorld;
	Camera mCam;

	sdl::Window& mWindow;

	GLuint mShadowMapShader, mGBufferGenShader, mDirLightingStencilShader, mDirLightingShader, mGlobalLightingShader,
	       mOutputSelectShader;
	ShadowMap mShadowMap;
	GBuffer mGBuffer;
	DirectionalLightingFramebuffer mDirLightFramebuffer;
	PostProcessFramebuffer mGlobalLightingFramebuffer, mOutputSelectFramebuffer;
	
	SSAO mSSAO;
	FullscreenQuadObject mFullscreenQuad;
	WorldRenderer mWorldRenderer;
	
	bool mOldWorldRenderer = false;
	int mRenderMode = 1;

	vec3 mCurrentVoxelPos; // TODO: Move this to CreationGameScreen
	Voxel mCurrentVoxel; // TODO: Move this to CreationGameScreen

	/*DirectionalLight mSun;
	vec3 mLightPosSpherical, mLightTarget;

	int currentLightAxis = 1;
	float lightCurrentSpeed = 1.0f;
	float lightNormalSpeed = 0.5f;
	float lightMaxSpeed = sfz::PI();*/

	vector<DirectionalLight> mLights;
	vector<DirectionalLightMesh> mLightMeshes;

	float mLightShaftExposure = 0.4f;

	float mFPSMean = 0.0f;
	long mFPSSamples = 0;
};

} // namespace vox

#endif