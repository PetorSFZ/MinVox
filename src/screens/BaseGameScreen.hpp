#pragma once
#ifndef VOX_SCREENS_BASE_GAME_SCREEN_HPP
#define VOX_SCREENS_BASE_GAME_SCREEN_HPP

#include "screens/Screen.hpp"

namespace vox {

class BaseGameScreen final : Screen {
public:
	virtual bool update(const sdl::GameController& ctrl, float delta) override final;
	virtual void render(float delta) override final;
};


} // namespace vox

#endif