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
	return ((mData >> 4) & 0x0F);
}

inline void Voxel::setType(VoxelType type)
{
	mData &= 0xF0; // Clears 4 lsb
	mData |= static_cast<uint8_t>(type); // Sets 4 lsb with type
}

inline void Voxel::setExtraData(uint8_t data)
{
	mData &= 0x0F; // Clears 4 msb
	mData |= ((data << 4) & 0xF0); // Sets 4 msb
}

} // namespace vox