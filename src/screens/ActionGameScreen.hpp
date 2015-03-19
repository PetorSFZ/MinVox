#pragma once
#ifndef VOX_SCREENS_ACTION_GAME_SCREEN_HPP
#define VOX_SCREENS_ACTION_GAME_SCREEN_HPP

#include "screens/BaseGameScreen.hpp"
#include "screens/CreationGameScreen.hpp"

namespace vox {

class ActionGameScreen final : public BaseGameScreen {
public:
	ActionGameScreen(sdl::Window& window, const std::string& worldName);

protected:
	void updateSpecific(const std::vector<SDL_Event>& events,
	                    const sdl::GameController& ctrl, float delta) override final;
};

} // namespace vox

#endif