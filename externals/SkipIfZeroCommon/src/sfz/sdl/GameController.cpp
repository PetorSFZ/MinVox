#include "sfz/sdl/GameController.hpp"

#include <sfz/Assert.hpp>

namespace sdl {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

static ButtonState* buttonPtr(GameController& c, uint8_t sdlButton)
{
	switch (static_cast<SDL_GameControllerButton>(sdlButton)) {
	case SDL_CONTROLLER_BUTTON_A: return &c.a;
	case SDL_CONTROLLER_BUTTON_B: return &c.b;
	case SDL_CONTROLLER_BUTTON_X: return &c.x;
	case SDL_CONTROLLER_BUTTON_Y: return &c.y;
	
	case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return &c.leftShoulder;
	case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return &c.rightShoulder;
	case SDL_CONTROLLER_BUTTON_LEFTSTICK: return &c.leftStickButton;
	case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return &c.rightStickButton;

	case SDL_CONTROLLER_BUTTON_DPAD_UP: return &c.padUp;
	case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return &c.padDown;
	case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return &c.padLeft;
	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return &c.padRight;

	case SDL_CONTROLLER_BUTTON_START: return &c.start;
	case SDL_CONTROLLER_BUTTON_BACK: return &c.back;
	case SDL_CONTROLLER_BUTTON_GUIDE: return &c.guide;
	default: return nullptr;
	}
}

static SDL_GameController* openGameController(int deviceIndex) noexcept
{
	if (!SDL_IsGameController(deviceIndex)) {
		std::cerr << "deviceIndex: " << deviceIndex << " is not a GameController." << std::endl;
		return nullptr;
	}

	SDL_GameController* ptr = SDL_GameControllerOpen(deviceIndex);
	if (ptr == NULL) {
		std::cerr << "Could not open GameController at device index " << deviceIndex << ", error: "
		          << SDL_GetError() << std::endl;
		return nullptr;
	}

	return ptr;
}

static int32_t getJoystickId(SDL_GameController* ptr) noexcept
{
	if (ptr == nullptr) return -1;

	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(ptr);
	if (joystick == NULL) {
		std::cerr << "Could not retrieve SDL_Joystick* from GameController, error: "
		          << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
	if (id < 0) {
		std::cerr << "Could not retrieve JoystickID from GameController, error: "
		          << SDL_GetError() << std::endl;
		return -1;
	}

	return id;
}

// GameController: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

GameControllerState GameController::state() const noexcept {
	GameControllerState copy;

	copy.a = this->a;
	copy.b = this->b;
	copy.x = this->x;
	copy.y = this->y;

	copy.stickDeadzone = this->stickDeadzone;
	copy.triggerDeadzone = this->triggerDeadzone;

	copy.leftStick = this->leftStick;
	copy.rightStick = this->rightStick;
	copy.leftStickButton = this->leftStickButton;
	copy.rightStickButton = this->rightStickButton;

	copy.leftShoulder = this->leftShoulder;
	copy.rightShoulder = this->rightShoulder;
	copy.leftTrigger = this->leftTrigger;
	copy.rightTrigger = this->rightTrigger;

	copy.padUp = this->padUp;
	copy.padDown = this->padDown;
	copy.padLeft = this->padLeft;
	copy.padRight = this->padRight;

	copy.start = this->start;
	copy.back = this->back;
	copy.guide = this->guide;

	return copy;
}

// GameController: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

GameController::GameController() noexcept
:
	mGameControllerPtr{nullptr},
	mID{-1}
{ }

GameController::GameController(GameController&& other) noexcept
{
	this->mGameControllerPtr = other.mGameControllerPtr;
	this->mID = other.mID;
	other.mGameControllerPtr = nullptr;
	other.mID = -1;
}

GameController& GameController::operator= (GameController&& other) noexcept
{
	this->mGameControllerPtr = other.mGameControllerPtr;
	this->mID = other.mID;
	other.mGameControllerPtr = nullptr;
	other.mID = -1;
	return *this;
}

GameController::GameController(int deviceIndex) noexcept
:
	mGameControllerPtr{openGameController(deviceIndex)},
	mID{getJoystickId(mGameControllerPtr)}
{ }

GameController::~GameController() noexcept
{
	if (mGameControllerPtr != nullptr) {
		SDL_GameControllerClose(mGameControllerPtr);
	}
}

// Update functions to update GameController struct
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

/** Starts the update process, should be called once before updateEvent() each frame. */
static void updateStart(GameController& controller) noexcept;

/** Updates state with an SDL Event, should be called once for each event after updateStart(). */
static void updateProcessEvent(GameController& controller, const SDL_Event& event) noexcept;

/** Finishes the update process, should be called once after all events have been processed. */
static void updateFinish(GameController& controller) noexcept;

void update(unordered_map<int32_t,GameController>& controllers, const vector<SDL_Event>& events) noexcept
{
	for (auto& c : controllers) updateStart(std::get<1>(c));

	for (const SDL_Event& event : events) {
		switch (event.type) {
		case SDL_CONTROLLERDEVICEADDED:
			// 'which' is the device index in this context
			{
				GameController c{event.cdevice.which};
				if (c.id() == -1) break;
				if (controllers.find(c.id()) != controllers.end()) break;
				controllers[c.id()] = std::move(c);
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			// 'which' is the joystick id in this context
			if (controllers.find(event.cdevice.which) != controllers.end()) {
				controllers.erase(event.cdevice.which);
			}
			break;
		case SDL_CONTROLLERDEVICEREMAPPED:
			// TODO: Nothing of value to do here?
			break;
			
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			if (controllers.find(event.cbutton.which) != controllers.end()) {
				updateProcessEvent(controllers[event.cbutton.which], event);
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			if (controllers.find(event.caxis.which) != controllers.end()) {
				updateProcessEvent(controllers[event.caxis.which], event);
			}
			break;

		default:
			// Do nothing
			break;
		}
	}

	for (auto& c : controllers) updateFinish(std::get<1>(c));
}

static void updateStart(GameController& c) noexcept
{
	// Changes previous DOWN state to HELD state.

	if (c.a == ButtonState::DOWN) c.a = ButtonState::HELD;
	if (c.b == ButtonState::DOWN) c.b = ButtonState::HELD;
	if (c.x == ButtonState::DOWN) c.x = ButtonState::HELD;
	if (c.y == ButtonState::DOWN) c.y = ButtonState::HELD;

	if (c.leftShoulder == ButtonState::DOWN) c.leftShoulder = ButtonState::HELD;
	if (c.rightShoulder == ButtonState::DOWN) c.rightShoulder = ButtonState::HELD;
	if (c.leftStickButton == ButtonState::DOWN) c.leftStickButton = ButtonState::HELD;
	if (c.rightStickButton == ButtonState::DOWN) c.rightStickButton = ButtonState::HELD;

	if (c.padUp == ButtonState::DOWN) c.padUp = ButtonState::HELD;
	if (c.padDown == ButtonState::DOWN) c.padDown = ButtonState::HELD;
	if (c.padLeft == ButtonState::DOWN) c.padLeft = ButtonState::HELD;
	if (c.padRight == ButtonState::DOWN) c.padRight = ButtonState::HELD;

	if (c.start == ButtonState::DOWN) c.start = ButtonState::HELD;
	if (c.back == ButtonState::DOWN) c.back = ButtonState::HELD;
	if (c.guide == ButtonState::DOWN) c.guide = ButtonState::HELD;

	// Changes previous UP state to NOT_PRESSED state.

	if (c.a == ButtonState::UP) c.a = ButtonState::NOT_PRESSED;
	if (c.b == ButtonState::UP) c.b = ButtonState::NOT_PRESSED;
	if (c.x == ButtonState::UP) c.x = ButtonState::NOT_PRESSED;
	if (c.y == ButtonState::UP) c.y = ButtonState::NOT_PRESSED;

	if (c.leftShoulder == ButtonState::UP) c.leftShoulder = ButtonState::NOT_PRESSED;
	if (c.rightShoulder == ButtonState::UP) c.rightShoulder = ButtonState::NOT_PRESSED;
	if (c.leftStickButton == ButtonState::UP) c.leftStickButton = ButtonState::NOT_PRESSED;
	if (c.rightStickButton == ButtonState::UP) c.rightStickButton = ButtonState::NOT_PRESSED;

	if (c.padUp == ButtonState::UP) c.padUp = ButtonState::NOT_PRESSED;
	if (c.padDown == ButtonState::UP) c.padDown = ButtonState::NOT_PRESSED;
	if (c.padLeft == ButtonState::UP) c.padLeft = ButtonState::NOT_PRESSED;
	if (c.padRight == ButtonState::UP) c.padRight = ButtonState::NOT_PRESSED;

	if (c.start == ButtonState::UP) c.start = ButtonState::NOT_PRESSED;
	if (c.back == ButtonState::UP) c.back = ButtonState::NOT_PRESSED;
	if (c.guide == ButtonState::UP) c.guide = ButtonState::NOT_PRESSED;
}

static void updateProcessEvent(GameController& controller, const SDL_Event& event) noexcept
{
	const float AXIS_MAX = 32766.0f; // Actual range [-32768, 32767]
	const float TRIGGER_MAX = 32766.0f; // Actual range [0, 32767]

	ButtonState* ptr = nullptr;
	switch (event.type) {
	case SDL_CONTROLLERBUTTONDOWN:
		/*std::cout << "Button down (" << event.cbutton.which << "): " << (int)event.cbutton.button << ", name: "
					  << SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event.cbutton.button)
					  << std::endl;*/
		ptr = buttonPtr(controller, event.cbutton.button);
		if (ptr != nullptr) *ptr = ButtonState::DOWN;
		break;
	case SDL_CONTROLLERBUTTONUP:
		/*std::cout << "Button up (" << event.cbutton.which << "): " << (int)event.cbutton.button << ", name: "
					  << SDL_GameControllerGetStringForButton((SDL_GameControllerButton)event.cbutton.button)
					  << std::endl;*/
		ptr = buttonPtr(controller, event.cbutton.button);
		if (ptr != nullptr) *ptr = ButtonState::UP;
		break;
	case SDL_CONTROLLERAXISMOTION:
		/*std::cout << "Axis " << (int)event.caxis.axis << ", name: "
					  << SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)event.caxis.axis)
					  << ", value: " << event.caxis.value << std::endl;*/
		
		const float axisVal = static_cast<float>(event.caxis.value);		
		switch (static_cast<SDL_GameControllerAxis>(event.caxis.axis)) {
		case SDL_CONTROLLER_AXIS_LEFTX:
			controller.leftStick[0] = axisVal/AXIS_MAX;
			break;
		case SDL_CONTROLLER_AXIS_LEFTY:
			controller.leftStick[1] = -axisVal/AXIS_MAX;
			break;
		case SDL_CONTROLLER_AXIS_RIGHTX:
			controller.rightStick[0] = axisVal/AXIS_MAX;
			break;
		case SDL_CONTROLLER_AXIS_RIGHTY:
			controller.rightStick[1] = -axisVal/AXIS_MAX;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
			controller.leftTrigger = axisVal/TRIGGER_MAX;
			break;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
			controller.rightTrigger = axisVal/TRIGGER_MAX;
			break;
		case SDL_CONTROLLER_AXIS_INVALID:
			std::cerr << "event.caxis.axis == SDL_CONTROLLER_AXIS_INVALID" << std::endl;
			break;
		case SDL_CONTROLLER_AXIS_MAX:
			std::cerr << "event.caxis.axis == SDL_CONTROLLER_AXIS_MAX" << std::endl;
			break;
		} 
		break;
	}
}

static void updateFinish(GameController& controller) noexcept
{
	// Deadzone checks
	if (sfz::length(controller.leftStick) < controller.stickDeadzone) {
		controller.leftStick[0] = 0.0f;
		controller.leftStick[1] = 0.0f;
	}
	if (sfz::length(controller.rightStick) < controller.stickDeadzone) {
		controller.rightStick[0] = 0.0f;
		controller.rightStick[1] = 0.0f;
	}
	if (controller.leftTrigger < controller.triggerDeadzone) {
		controller.leftTrigger = 0.0f;
	}
	if (controller.rightTrigger < controller.triggerDeadzone) {
		controller.rightTrigger = 0.0f;
	}

	// Normalize if norm is > 1
	if (sfz::length(controller.leftStick) > 1.0f) {
		controller.leftStick = sfz::normalize(controller.leftStick);
	}
	if (sfz::length(controller.rightStick) > 1.0f) {
		controller.rightStick = sfz::normalize(controller.rightStick);
	}
	if (controller.leftTrigger > 1.0f) {
		controller.leftTrigger = 1.0f;
	}
	if (controller.rightTrigger > 1.0f) {
		controller.rightTrigger = 1.0f;
	}
}

} // namespace sdl