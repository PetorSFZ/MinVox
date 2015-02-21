namespace vox {

inline ChunkOffset::ChunkOffset(int y, int z, int x)
:
	mY{y},
	mZ{z},
	mX{x}
{ }

inline void ChunkOffset::set(int y, int z, int x)
{
	mY = y;
	mZ = z;
	mX = x;
}

inline std::string ChunkOffset::to_string() const
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

inline bool ChunkOffset::operator== (const ChunkOffset& other)
{
	return mY == other.mY && mZ == other.mZ && mX == other.mX;
}

inline bool ChunkOffset::operator!= (const ChunkOffset& other)
{
	return !((*this) == other);
}

} // namespace vox