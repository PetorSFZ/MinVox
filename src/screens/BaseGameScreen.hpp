#pragma once
#ifndef VOX_SCREENS_BASE_GAME_SCREEN_HPP
#define VOX_SCREENS_BASE_GAME_SCREEN_HPP

#include <string>
#include <memory>

#include "sfz/GL.hpp"

#include "Model.hpp"
#include "Rendering.hpp"
#include "screens/IScreen.hpp"

namespace vox {

class BaseGameScreen : public IScreen {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	BaseGameScreen() = delete;
	//BaseGameScreen(const BaseGameScreen&) = delete;
	BaseGameScreen& operator= (const BaseGameScreen&) = delete;

	BaseGameScreen(sdl::Window& window, const std::string& worldName);
	//BaseGameScreen(const BaseGameScreen& baseGameScreen);
	virtual ~BaseGameScreen();

	// Overriden methods from IScreen
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void update(const std::vector<SDL_Event>& events,
	            const sdl::GameController& ctrl, float delta) override final;
	std::unique_ptr<IScreen> changeScreen() override final;
	bool quit() override final;
	void render(float delta) override final;

protected:
	// Protected methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void quitApplication();

	void changeScreen(IScreen* newScreen);

	// Protected members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	World mWorld;
	Camera mCam;

	sdl::Window& mWindow;
	Assets mAssets;

	GLuint mShaderProgram, mShadowMapShaderProgram, mPostProcessShaderProgram;
	BigFramebuffer mBaseFramebuffer;
	Framebuffer mPostProcessedFramebuffer;

	sfz::mat4f projMatrix;
	sfz::vec3f lightPosSpherical{60.0f, sfz::g_PI_FLOAT*0.15f, sfz::g_PI_FLOAT*0.35f}; // [0] = r, [1] = theta, [2] = phi
	sfz::vec3f lightTarget{16.0f, 0.0f, 16.0f};
	sfz::vec3f lightColor{1.0f, 1.0f, 1.0f};
	int currentLightAxis = 1;
	float lightCurrentSpeed = 1.0f;
	float lightNormalSpeed = 0.5f;
	float lightMaxSpeed = sfz::g_PI_FLOAT;

private:
	// Private methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void reloadFramebuffers(int width, int height);

	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	IScreen* mNewScreenPtr = nullptr;
	bool mQuit = false;
};

} // namespace vox

#endif