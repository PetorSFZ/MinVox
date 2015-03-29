#include "screens/ActionGameScreen.hpp"

namespace vox {

ActionGameScreen::ActionGameScreen(sdl::Window& window, const std::string& worldName)
:
	BaseGameScreen{window, worldName}
{
	
}

void ActionGameScreen::updateSpecific(const std::vector<SDL_Event>& events,
                                      const sdl::GameController& ctrl, float delta)
{
	if (ctrl.mButtonY == sdl::Button::UP) {
		changeScreen(new CreationGameScreen(mWindow, mWorld.mName));
	}
}

} // namespace vox