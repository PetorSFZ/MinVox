#pragma once
#ifndef SFZ_GL_TEXTURE_PACKER_HPP
#define SFZ_GL_TEXTURE_PACKER_HPP

#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/Texture.hpp>
#include <sfz/gl/TextureRegion.hpp>

#include <cstddef> // size_t
#include <vector>
#include <string>

#include <sfz/MSVC12HackON.hpp>

namespace gl {

using std::size_t;
using std::vector;
using std::string;

using sfz::TextureRegion;

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

	TexturePacker(const string& dirPath, const vector<string>& filenames, int padding = 1,
	              size_t suggestedWidth = 256, size_t suggestedHeight = 256) noexcept;
	~TexturePacker() noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	inline GLuint texture() const noexcept { return mTexture; }

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	size_t mSize;
	vector<string> mFilenames;
	GLuint mTexture;
	size_t mTexWidth, mTexHeight;
	vector<TextureRegion> mTexRegions;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif
