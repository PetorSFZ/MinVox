#pragma once
#ifndef SFZ_GL_TEXTURE_HPP
#define SFZ_GL_TEXTURE_HPP

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>
#include <string>
#include <cstring> // std::memcpy
#include <iostream>
#include <exception> // std::terminate
#include <algorithm> // std::swap
#include "sfz/gl/Utils.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace gl {

class Texture final {
public:
	// Public members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	
	GLuint mHandle;
	
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	Texture() = delete;
	Texture(const Texture&) = delete;
	Texture& operator= (const Texture&) = delete;
	
	Texture(const std::string& path) noexcept;
	Texture& operator= (Texture&& other) noexcept;
	~Texture() noexcept;
};

} // namespace gl

#include <sfz/MSVC12HackOFF.hpp>
#endif