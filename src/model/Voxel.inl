namespace vox {

inline Voxel::Voxel()
:
	mData{0}
{
	// Defaults to air with no additional data.
}

inline Voxel::Voxel(VoxelType type, uint8_t data)
{
	setType(type);
	setExtraData(data);	
}

inline VoxelType Voxel::type() const
{
	return static_cast<VoxelType>(mData & 0x07);
}

inline uint8_t Voxel::extraData() const
{
	return ((mData >> 3) & 0x1F);
}

inline void Voxel::setType(VoxelType type)
{
	mData &= 0xF8; // Clears 3 lsb
	mData |= static_cast<uint8_t>(type); // Sets 3 lsb with type
}

inline void Voxel::setExtraData(uint8_t data)
{
	mData &= 0x07; // Clears 5 msb
	mData |= ((data << 3) & 0xF8); // Sets 5 msb
}

} // namespace vox