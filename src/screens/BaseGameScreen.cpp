#include "screens/BaseGameScreen.hpp"

namespace vox {

void BaseGameScreen::update(const std::vector<SDL_Event>& events,
                            const sdl::GameController& ctrl, float delta)
{

}

std::unique_ptr<IScreen> BaseGameScreen::changeScreen()
{
	return std::move(std::unique_ptr<IScreen>{});
}

bool BaseGameScreen::quit()
{
	return false;
}

void BaseGameScreen::render(float delta)
{

}

} // namespace vox