#pragma once
#ifndef VOX_SCREENS_CREATION_GAME_SCREEN_HPP
#define VOX_SCREENS_CREATION_GAME_SCREEN_HPP

#include "screens/BaseGameScreen.hpp"
#include "screens/ActionGameScreen.hpp"

namespace vox {

class CreationGameScreen final : public BaseGameScreen {
public:
	CreationGameScreen(sdl::Window& window, const std::string& worldName);

protected:
	void updateSpecific(const std::vector<SDL_Event>& events,
	                    const sdl::GameController& ctrl, float delta) override final;
};

} // namespace vox

#endif