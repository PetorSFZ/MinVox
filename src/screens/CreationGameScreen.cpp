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
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				mCam.mPos += (-right * 25.0f * delta);}
				break;
			case 'd':
			case 'D':
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				mCam.mPos += (right * 25.0f * delta);}
				break;
			case 'q':
			case 'Q':
				mCam.mPos += (sfz::vec3{0.0f,-1.0f,0.0} * 25.0f * delta);
				break;
			case 'e':
			case 'E':
				mCam.mPos += (sfz::vec3{0.0f,1.0f,0.0} * 25.0f * delta);
				break;
			case 'p':
			case 'P':
				mOldWorldRenderer = !mOldWorldRenderer;
				if (mOldWorldRenderer) std::cout << "Using old (non-meshed) world renderer.\n";
				else std::cout << "Using (meshed) world renderer.\n";
				break;
			case SDLK_UP:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 1.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_DOWN:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 0.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, -1.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_LEFT:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, -1.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_RIGHT:
				{sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
				sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, 1.0f*sfz::PI()*delta);
				sfz::mat3 yTurn = sfz::rotationMatrix3(right, 0.0f*sfz::PI()*delta);
				mCam.mDir = (yTurn * xTurn * mCam.mDir);
				mCam.mUp = (yTurn * xTurn * mCam.mUp);}
				break;
			case SDLK_SPACE:
				vec3 pos = mCam.mPos + mCam.mDir * 1.5f;
				Voxel v = mWorld.getVoxel(pos);
				if (v.mType != VOXEL_AIR) mWorld.setVoxel(pos, Voxel{VOXEL_AIR});
				else mWorld.setVoxel(pos, Voxel{VOXEL_ORANGE});
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
		
	}

	// Analogue Sticks
	if (sfz::length(ctrl.mRightStick) > ctrl.mRightStickDeadzone) {
		sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
		sfz::mat3 xTurn = sfz::rotationMatrix3(sfz::vec3{0.0f,-1.0f,0.0f}, ctrl.mRightStick[0]*turningSpeed*delta);
		sfz::mat3 yTurn = sfz::rotationMatrix3(right, ctrl.mRightStick[1]*turningSpeed*delta);
		mCam.mDir = (yTurn * xTurn * mCam.mDir);
		mCam.mUp = (yTurn * xTurn * mCam.mUp);
	}
	if (sfz::length(ctrl.mLeftStick) > ctrl.mLeftStickDeadzone) {
		sfz::vec3 right = sfz::normalize(sfz::cross(mCam.mDir, mCam.mUp));
		mCam.mPos += ((mCam.mDir * ctrl.mLeftStick[1] + right * ctrl.mLeftStick[0]) * currentSpeed * delta);
	}

	// Control Pad
	if (ctrl.mButtonDPADUp == sdl::Button::DOWN) {
		mLightShaftExposure += 0.05f;
		if (mLightShaftExposure > 1.0f) mLightShaftExposure = 1.0f;
		std::cout << "Light shaft exposure: " << mLightShaftExposure << std::endl;
	} else if (ctrl.mButtonDPADDown == sdl::Button::DOWN) {
		mLightShaftExposure -= 0.05f;
		if (mLightShaftExposure < 0.0f) mLightShaftExposure = 0.0f;
		std::cout << "Light shaft exposure: " << mLightShaftExposure << std::endl;
	} else if (ctrl.mButtonDPADLeft == sdl::Button::DOWN) {
		if (mCurrentVoxel.mType >= 1) {
			mCurrentVoxel = Voxel(mCurrentVoxel.mType - uint8_t(1));
		}
	} else if (ctrl.mButtonDPADRight == sdl::Button::DOWN) {
		if (mCurrentVoxel.mType < Assets::INSTANCE().numVoxelTypes() - 1) {
			mCurrentVoxel = Voxel(mCurrentVoxel.mType + uint8_t(1));
		}
	}

	// Shoulder buttons
	if (ctrl.mButtonLeftShoulder == sdl::Button::DOWN || ctrl.mButtonLeftShoulder == sdl::Button::HELD) {
		mCam.mPos -= (sfz::vec3{0,1,0} * currentSpeed * delta);
	}
	else if (ctrl.mButtonRightShoulder == sdl::Button::DOWN || ctrl.mButtonRightShoulder == sdl::Button::HELD) {
		mCam.mPos += (sfz::vec3{0,1,0} * currentSpeed * delta);
	}

	auto vPos = mCam.mPos + mCam.mDir * 4.0f;
	mCurrentVoxelPos = vec3{std::floorf(vPos[0]), std::floorf(vPos[1]), std::floorf(vPos[2])};

	// Face buttons
	if (ctrl.mButtonY == sdl::Button::UP) {
		std::random_device rd;
		std::mt19937_64 gen{rd()};
		std::uniform_real_distribution<float> distr{0.0f, 1.0f};
		mLights.emplace_back(mCam.mPos, mCam.mDir, 0.5f, 40.0f, vec3{distr(gen), distr(gen), distr(gen)});
		mLightMeshes.emplace_back(mLights.back().mCam.mVerticalFov, mLights.back().mCam.mNear, mLights.back().mCam.mFar);
		std::cout << "Light: Pos: " << mLights.back().mCam.mPos << ", Dir: " << mLights.back().mCam.mDir << ", Color: " << mLights.back().mColor << std::endl;
	}
	if (ctrl.mButtonX == sdl::Button::UP) {
		if (mLights.size() > 0) {
			mLights.pop_back();
			mLightMeshes.pop_back();
		}
	}
	if (ctrl.mButtonB == sdl::Button::UP) {
		mWorld.setVoxel(mCurrentVoxelPos, Voxel{VOXEL_AIR});
	}
	if (ctrl.mButtonA == sdl::Button::UP) {
		mWorld.setVoxel(mCurrentVoxelPos, mCurrentVoxel);
	}

	// Menu buttons
	if (ctrl.mButtonBack == sdl::Button::UP) {
		quitApplication();
	}
	if (ctrl.mButtonStart == sdl::Button::UP) {
		changeScreen(new ActionGameScreen(mWindow, mWorld.mName));
	}
}

} // namespace vox