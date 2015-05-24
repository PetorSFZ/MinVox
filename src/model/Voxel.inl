#include <sfz/MSVC12HackON.hpp>

namespace vox {

inline Voxel::Voxel() noexcept
:
	mType{0}
{
	static_assert(sizeof(Voxel) == 1, "Voxel is padded.");
}

inline Voxel::Voxel(uint8_t type) noexcept
:
	mType{type}
{
	static_assert(sizeof(Voxel) == 1, "Voxel is padded.");
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>