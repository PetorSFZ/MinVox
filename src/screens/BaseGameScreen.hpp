#pragma once
#ifndef VOX_SCREENS_BASE_GAME_SCREEN_HPP
#define VOX_SCREENS_BASE_GAME_SCREEN_HPP

#include "screens/IScreen.hpp"

namespace vox {

class BaseGameScreen final : public IScreen {
public:
	virtual void update(const std::vector<SDL_Event>& events,
	                    const sdl::GameController& ctrl, float delta) override final;
	virtual std::unique_ptr<IScreen> changeScreen() override final;
	virtual bool quit() override final;
	virtual void render(float delta) override final;
};


} // namespace vox

#endif