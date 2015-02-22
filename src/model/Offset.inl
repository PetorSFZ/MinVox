namespace vox {

inline Offset::Offset(int y, int z, int x)
:
	mY{y},
	mZ{z},
	mX{x}
{ }

inline void Offset::set(int y, int z, int x)
{
	mY = y;
	mZ = z;
	mX = x;
}

inline sfz::vec3f Offset::toVector() const
{
	return sfz::vec3f{(float)mX, (float)mY, (float)mZ};
}

inline std::string Offset::to_string() const
{
	std::string str;
	str += "y: ";
	str += std::to_string(mY);
	str += ", z: ";
	str += std::to_string(mZ);
	str += ", x: ";
	str += std::to_string(mX);
	return std::move(str);
}

inline bool Offset::operator== (const Offset& other)
{
	return mY == other.mY && mZ == other.mZ && mX == other.mX;
}

inline bool Offset::operator!= (const Offset& other)
{
	return !((*this) == other);
}

inline Offset chunkToVoxelOffset(const Offset& offset, int chunkSize)
{
	return Offset{offset.mY*chunkSize, offset.mZ*chunkSize, offset.mX*chunkSize};
}

} // namespace vox