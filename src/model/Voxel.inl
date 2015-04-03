#include <sfz/MSVC12HackON.hpp>

namespace vox {

inline Voxel::Voxel() noexcept
:
	mData{0}
{
	// Defaults to air with no additional data.
}

inline Voxel::Voxel(uint8_t rawVoxel) noexcept
:
	mData{rawVoxel}
{ }

inline Voxel::Voxel(VoxelType type) noexcept
:
	mData{static_cast<uint8_t>(type)}
{ }

inline Voxel::Voxel(VoxelType type, uint8_t data) noexcept
{
	setType(type);
	setExtraData(data);	
}

inline VoxelType Voxel::type() const noexcept
{
	return static_cast<VoxelType>(mData & 0x07);
}

inline uint8_t Voxel::extraData() const noexcept
{
	return static_cast<uint8_t>(((mData >> 4) & 0x0F));
}

inline void Voxel::setType(VoxelType type) noexcept
{
	mData &= 0xF0; // Clears 4 lsb
	mData |= static_cast<uint8_t>(type); // Sets 4 lsb with type
}

inline void Voxel::setExtraData(uint8_t data) noexcept
{
	mData &= 0x0F; // Clears 4 msb
	mData |= ((data << 4) & 0xF0); // Sets 4 msb
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>