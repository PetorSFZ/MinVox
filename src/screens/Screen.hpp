#pragma once
#ifndef VOX_SCREEN_HPP
#define VOX_SCREEN_HPP

#include "sfz/sdl/GameController.hpp"

namespace vox {

class Screen {
public:
	virtual bool update(const sdl::GameController& ctrl, float delta);
	virtual void render(float delta);
};

} // namespace vox

#endif