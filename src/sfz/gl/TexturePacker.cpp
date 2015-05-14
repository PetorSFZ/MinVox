#include "sfz/gl/TexturePacker.hpp"

//#define STB_RECT_PACK_IMPLEMENTATION
#include "sfz/gl/font/stb_rect_pack.h"

#include "sfz/gl/Utils.hpp"
#include <SDL_image.h>
#include <new> // std::nothrow
#include <cstring> // std::memcpy
#include <iostream>
#include <exception> // std::terminate
#include <algorithm> // std::swap

#include <sfz/MSVC12HackON.hpp>

namespace gl {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

// Simple function that flips a surface by byte-level manipulation.
void flipSurface(SDL_Surface* surface) noexcept
{
	// Locking the surface
	if (SDL_LockSurface(surface) < 0) {
		std::cerr << "Couldn't lock surface for image flipping: " << SDL_GetError() << std::endl;
	}

	// Constants
	const int w = surface->w;
	const int h = surface->h;
	const int pixelCount = w * h;
	const int bytesPerPixel = surface->format->BytesPerPixel;
	const int bytesPerRow = w * bytesPerPixel;
	const int totalByteCount = pixelCount * bytesPerPixel;
	Uint32* const surfacePixels = static_cast<Uint32*>(surface->pixels);
	Uint32* const pixelBuffer = new Uint32[pixelCount];

	// surfPtr reads from surface, starting value is first pixel.
	Uint32* surfPtr = surfacePixels;

	// bufPtr writes to buffer, starting value is the first pixel outside the image
	Uint32* bufPtr = pixelBuffer + pixelCount;

	// Copy pixels from surface to buffer until all pixels are copied
	while (bufPtr > pixelBuffer) {
		bufPtr = bufPtr - w; // Move bufPtr back one image row
		std::memcpy(bufPtr, surfPtr, bytesPerRow); // Copy one image row to buffer
		surfPtr = surfPtr + w; // Move surfPtr forward one image row
	}

	// Copy pixels back from buffer and free memory.
	std::memcpy(surfacePixels, pixelBuffer, totalByteCount);
	delete[] pixelBuffer;

	// Unlocking the surface
	SDL_UnlockSurface(surface);
}

SDL_Surface* loadTexture(const std::string& path) noexcept
{
	// Load specified surface.
	SDL_Surface* surface = NULL;
	surface = IMG_Load(path.c_str());
	if (surface == NULL) {
		std::cerr << "Unable to load image at: " << path << ", error: " << IMG_GetError();
		std::terminate();
	}

	// Some error checking.
	if (surface->format->BytesPerPixel != 4) {
		std::cerr << "Image at \"" << path << "\" doesn't have 4 bytes per pixel." << std::endl;
		std::terminate();
	}
	if (!SDL_ISPIXELFORMAT_ALPHA(surface->format->format)) {
		std::cerr << "Image at \"" << path << "\" doesn't contain alpha channel." << std::endl;
		std::terminate();
	}

	// Flips surface so UV coordinates will be in a right-handed system in OpenGL.
	flipSurface(surface);

	return surface;
}

bool packRects(vector<stbrp_rect>& rects, int width, int height) noexcept
{
	stbrp_context packContext;
	stbrp_node* nodes = new (std::nothrow) stbrp_node[width+2];
	stbrp_init_target(&packContext, width, height, nodes, width);
	stbrp_pack_rects(&packContext, rects.data(), rects.size());
	delete[] nodes;
	
	// Check if all rects were packed
	for (auto& rect : rects) {
		if (!rect.was_packed) return false;
	}
	return true;
}

} // anonymous namespace

// TexturePacker: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

TexturePacker::TexturePacker(const string& dirPath, const vector<string>& filenames, int padding,
                             size_t suggestedWidth, size_t suggestedHeight) noexcept
:
	mSize{filenames.size()},
	mFilenames{filenames},
	mTexRegions{mSize}
{
	vector<SDL_Surface*> surfaces;
	vector<stbrp_rect> rects;
	for (auto& filename : filenames) {
		surfaces.emplace_back(loadTexture(dirPath + filename));
		struct stbrp_rect r;
		r.id = surfaces.size()-1;
		r.w = surfaces.back()->w + 2*padding;
		r.h = surfaces.back()->h + 2*padding;
		rects.push_back(r);
	}

	size_t width = suggestedWidth;
	size_t height = suggestedHeight;

	packRects(rects, (int)width, (int)height);

	// Little endian masks
	uint32_t rmask = 0x000000ff;
    uint32_t gmask = 0x0000ff00;
    uint32_t bmask = 0x00ff0000;
    uint32_t amask = 0xff000000;
    SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);

	// Blitting individual surfaces to common surface and calculating TextureRegions
	vec2f texDimInv{1.0f/(float)width, 1.0f/(float)height};
	for (size_t i = 0; i < mSize; ++i) {
		SDL_Rect dstRect;
		dstRect.w = surfaces[i]->w - 2*padding;
		dstRect.h = surfaces[i]->h - 2*padding;
		dstRect.x = rects[i].x + padding;
		dstRect.y = rects[i].y + padding;

		SDL_BlitSurface(surfaces[i], NULL, surface, &dstRect);

		// Calculate TextureRegion
		vec2f min = vec2f{(float)(dstRect.x + padding), (float)(dstRect.y + padding)}
		            .elemMult(texDimInv);
		vec2f max = vec2f{(float)(dstRect.x + dstRect.w - 2*padding),
		                  (float)(dstRect.y + dstRect.h - 2*padding)}
		            .elemMult(texDimInv);
		mTexRegions[i] = TextureRegion{min, max};
		/*std::cout << filenames[i] << ": " << dstRect.x << "x, " << dstRect.y << "y, " << dstRect.w << "w, " << dstRect.h << "h\n";
		std::cout << filenames[i] << ": min=" << mTexRegions[i].mUVMin << ", max=" << mTexRegions[i].mUVMax << std::endl;*/
	}

	// Cleaning up surfaces
	for (SDL_Surface* surface : surfaces) {
		SDL_FreeSurface(surface);
	}

	// Generating texture.
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
	             surface->pixels);
	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Enable anisotropic filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

	SDL_FreeSurface(surface);
}

TexturePacker::~TexturePacker() noexcept
{
	glDeleteTextures(1, &mTexture);
}

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>