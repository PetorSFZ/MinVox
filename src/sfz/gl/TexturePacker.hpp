#pragma once
#ifndef SFZ_GL_TEXTURE_PACKER_HPP
#define SFZ_GL_TEXTURE_PACKER_HPP

#include <sfz/MSVC12HackON.hpp>

namespace sfz {

// TexturePacker
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class TexturePacker final {
public:
	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	TexturePacker() = delete;
	TexturePacker(const TexturePacker&) = delete;
	TexturePacker(TexturePacker&&) = delete;
	TexturePacker& operator= (const TexturePacker&) = delete;
	TexturePacker& operator= (TexturePacker&&) = delete;

	TexturePacker(const char** str) noexcept;

private:


};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif
