#pragma once
#ifndef VOX_SCREEN_HPP
#define VOX_SCREEN_HPP

#include <memory>
#include <vector>

#include "sfz/SDL.hpp"

namespace vox {

/** @brief Common interface for each screen. */
class IScreen {
public:
	/** @brief Called once every frame, update logic should happen here. */
	virtual void update(const std::vector<SDL_Event>& events,
	                    const sdl::GameController& ctrl, float delta) = 0;

	/** @brief Called once very frame after update(), return nullptr to keep current screen. */
	virtual std::unique_ptr<IScreen> changeScreen() = 0;

	/** @brief Called once every frame after changeScreen(), return true to quit program. */
	virtual bool quit() = 0;

	/** @brief Called once every frame after quit(), rendering should happen here. */
	virtual void render(float delta) = 0;
};

} // namespace vox

#endif