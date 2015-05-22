#include "screens/CreationGameScreen.hpp"

namespace vox {

CreationGameScreen::CreationGameScreen(sdl::Window& window, const std::string& worldName)
:
	BaseGameScreen{window, worldName}
{

}

void CreationGameScreen::updateSpecific(const std::vector<SDL_Event>& events,
                                        const sdl::GameController& ctrl, float delta)
{
	for (auto& event : events) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE: quitApplication(); return;
			case 'w':
			case 'W':
				mCam.mPos += (mCam.mDir * 25.0f * delta);
				break;
			case 's':
			case 'S':
				mCam.mPos -= (mCam.mDir * 25.0f * delta);
				break;
			case 'a':
			case 'A':
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				mCam.mPos += (-right * 25.0f * delta);}
				break;
			case 'd':
			case 'D':
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				mCam.mPos += (right * 25.0f * delta);}
				break;
			case 'q':
			case 'Q':
				mCam.mPos += (sfz::vec3f{0.0f,-1.0f,0.0} * 25.0f * delta);
				break;
			case 'e':
			case 'E':
				mCam.mPos += (sfz::vec3f{0.0f,1.0f,0.0} * 25.0f * delta);
				break;
			case 'p':
			case 'P':
				mOldWorldRenderer = !mOldWorldRenderer;
				if (mOldWorldRenderer) std::cout << "Using old (non-meshed) world renderer.\n";
				else std::cout << "Using (meshed) world renderer.\n";
				break;
			case SDLK_UP:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 1.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_DOWN:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, -1.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_LEFT:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, -1.0f*sfz::PI()*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_RIGHT:
				{sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
				sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, 1.0f*sfz::PI()*delta);
				sfz::mat3f yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			}
			break;
		}
	}

	float currentSpeed = 3.0f;
	float turningSpeed = sfz::PI();

	// Triggers
	if (ctrl.mLeftTrigger > ctrl.mLeftTriggerDeadzone) {
		currentSpeed += (ctrl.mLeftTrigger * 12.0f);
	}
	if (ctrl.mRightTrigger > ctrl.mRightTriggerDeadzone) {
		lightCurrentSpeed = ctrl.mRightTrigger * lightMaxSpeed;
	} else {
		lightCurrentSpeed = lightNormalSpeed;
	}

	// Analogue Sticks
	if (ctrl.mRightStick.norm() > ctrl.mRightStickDeadzone) {
		sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
		sfz::mat3f xTurn = sfz::rotationMatrix3(sfz::vec3f{0.0f,-1.0f,0.0f}, ctrl.mRightStick[0]*turningSpeed*delta);
		sfz::mat3f yTurn = sfz::rotationMatrix3(right, ctrl.mRightStick[1]*turningSpeed*delta);
		mCam.mDir = (yTurn * xTurn * mCam.mDir);
		mCam.mUp = (yTurn * xTurn * mCam.mUp);
	}
	if (ctrl.mLeftStick.norm() > ctrl.mLeftStickDeadzone) {
		sfz::vec3f right = sfz::cross(mCam.mDir, mCam.mUp).normalize();
		mCam.mPos += ((mCam.mDir * ctrl.mLeftStick[1] + right * ctrl.mLeftStick[0]) * currentSpeed * delta);
	}

	// Shoulder buttons
	if (ctrl.mButtonLeftShoulder == sdl::Button::DOWN || ctrl.mButtonLeftShoulder == sdl::Button::HELD) {
		mCam.mPos -= (sfz::vec3f{0,1,0} * currentSpeed * delta);
	}
	else if (ctrl.mButtonRightShoulder == sdl::Button::DOWN || ctrl.mButtonRightShoulder == sdl::Button::HELD) {
		mCam.mPos += (sfz::vec3f{0,1,0} * currentSpeed * delta);
	}

	// Face buttons
	if (ctrl.mButtonY == sdl::Button::UP) {
		currentLightAxis = -1;
	}
	if (ctrl.mButtonX == sdl::Button::UP) {
		currentLightAxis = 1;
	}
	if (ctrl.mButtonB == sdl::Button::UP) {
		currentLightAxis = 2;
	}
	if (ctrl.mButtonA == sdl::Button::UP) {
		vec3f pos = mCam.mPos + mCam.mDir * 1.5f;
		Voxel v = mWorld.getVoxel(pos);
		if (v.type() != VoxelType::AIR) mWorld.setVoxel(pos, Voxel{VoxelType::AIR, 0});
		else mWorld.setVoxel(pos, Voxel{VoxelType::ORANGE, 0});
	}

	// Menu buttons
	if (ctrl.mButtonBack == sdl::Button::UP) {
		mOldWorldRenderer = !mOldWorldRenderer;
		if (mOldWorldRenderer) std::cout << "Using old (linear) world renderer.\n";
		else std::cout << "Using (recursive) world renderer.\n";
	}
	if (ctrl.mButtonStart == sdl::Button::UP) {
		changeScreen(new ActionGameScreen(mWindow, mWorld.mName));
	}
}

} // namespace vox