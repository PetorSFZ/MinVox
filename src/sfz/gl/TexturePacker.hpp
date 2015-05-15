#pragma once
#ifndef SFZ_GL_TEXTURE_PACKER_HPP
#define SFZ_GL_TEXTURE_PACKER_HPP

#include <sfz/gl/OpenGL.hpp>
#include <sfz/gl/Texture.hpp>
#include <sfz/gl/TextureRegion.hpp>

#include <cstddef> // size_t
#include <vector>
#include <string>
#include <unordered_map>

#include <sfz/MSVC12HackON.hpp>

namespace gl {

using std::size_t;
using std::vector;
using std::string;
using std::unordered_map;

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
	inline size_t textureWidth() const noexcept { return mTexWidth; }
	inline size_t textureHeight() const noexcept { return mTexHeight; }
	inline const vector<string>& filenames() const noexcept { return mFilenames; }
	const TextureRegion* textureRegion(const string& filename) const noexcept;

private:
	// Private members
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	GLuint mTexture;
	size_t mTexWidth, mTexHeight;
	vector<string> mFilenames;
	unordered_map<string, TextureRegion> mTextureRegionMap;
};

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>
#endif
