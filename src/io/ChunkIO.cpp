#include "io/ChunkIO.hpp"

#include <sfz/util/IO.hpp>

namespace vox {

namespace {

std::string directoryPath(const std::string& worldName)
{
	std::string directoryPath = basePath() + worldName + "/";
	return std::move(directoryPath);
}

std::string filename(int xOffset, int yOffset, int zOffset, const std::string& worldName)
{
	std::string filePath = directoryPath(worldName) + "chunk__" + std::to_string(xOffset) + "x_"
	                     + std::to_string(yOffset) + "y_" + std::to_string(zOffset) + "z.bin";
	return std::move(filePath);
}

} // anonymous namespace

bool readChunk(Chunk& chunk, int xOffset, int yOffset, int zOffset, const std::string& worldName)
{
	static const size_t VOXELS_PER_CHUNK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;
	std::string filePath = filename(xOffset, yOffset, zOffset, worldName);
	if (!sfz::directoryExists(filePath.c_str())) return false;

	std::FILE* chunkFile = fopen(filePath.c_str(), "rb");

	size_t readCount = fread(chunk.mChunkPart8s, sizeof(Voxel), VOXELS_PER_CHUNK, chunkFile);
	fclose(chunkFile);

	if (readCount != VOXELS_PER_CHUNK) {
		std::cerr << "Could only read " << readCount << " of " << VOXELS_PER_CHUNK
		          << " voxels from file: " << filePath << std::endl;
		return false;
	}

	return true;
}

bool writeChunk(Chunk& chunk, int xOffset, int yOffset, int zOffset, const std::string& worldName)
{
	static const size_t VOXELS_PER_CHUNK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;
	std::string dirPath = directoryPath(worldName);
	std::string filePath = filename(xOffset, yOffset, zOffset, worldName);

	// Make sure directory exists
	if (!sfz::directoryExists(dirPath.c_str())) {
		if(!sfz::createDirectory(dirPath.c_str())) {
			std::cerr << "Couldn't create directory: \"" << dirPath << "\", can't write chunk: \""
			          << filePath << "\"" << std::endl;
			return false;
		}
	}

	std::FILE* chunkFile = fopen(filePath.c_str(), "wb");
	if (chunkFile == NULL) return false;

	size_t writeCount = fwrite(chunk.mChunkPart8s, sizeof(Voxel), VOXELS_PER_CHUNK, chunkFile);
	fclose(chunkFile);

	if (writeCount != VOXELS_PER_CHUNK) {
		std::cerr << "Could only write " << writeCount << " of " << VOXELS_PER_CHUNK
		          << " voxels to file: " << filePath << std::endl;
		return false;
	}
	
	return true;
}

} // namespace vox