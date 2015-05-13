#include "sfz/gl/TexturePacker.hpp"

#include <SDL.h>
#include "sfz/gl/Utils.hpp"
//#define STB_RECT_PACK_IMPLEMENTATION
#include "sfz/gl/font/stb_rect_pack.h"

#include <new> // std::nothrow

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

TexturePacker::TexturePacker(const string& dirPath, const vector<string>& filenames,
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
		r.w = surfaces.back()->w;
		r.h = surfaces.back()->h;
		rects.push_back(r);
	}

	size_t width = suggestedWidth;
	size_t height = suggestedHeight;

	packRects(rects, (int)width, (int)height);

	

	// Cleaning up surfaces
	for (SDL_Surface* surface : surfaces) {
		SDL_FreeSurface(surface);
	}

	glGenTextures(1, &mTexture);
}

TexturePacker::~TexturePacker() noexcept
{
	glDeleteTextures(1, &mTexture);
}

} // namespace sfz

#include <sfz/MSVC12HackOFF.hpp>