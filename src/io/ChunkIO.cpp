#include "io/ChunkIO.hpp"

namespace vox {

namespace {

std::string directoryPath(const std::string& worldName)
{
	std::string directoryPath = basePath() + worldName + "/";
	return std::move(directoryPath);
}

std::string filename(int yOffset, int zOffset, int xOffset, const std::string& worldName)
{
	std::string filePath = directoryPath(worldName) + "chunk" + std::to_string(yOffset) + "y"
	                     + std::to_string(zOffset) + "z" + std::to_string(xOffset) + "x.bin";
	return std::move(filePath);
}

} // anonymous namespace

bool readChunk(Chunk& chunk, int yOffset, int zOffset, int xOffset, const std::string& worldName)
{
	static const size_t VOXELS_PER_CHUNK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;
	std::string filePath = filename(yOffset, zOffset, xOffset, worldName);
	if (!exists(filePath)) return false;

	std::FILE* chunkFile = fopen(filePath.c_str(), "rb");

	size_t readCount = fread(chunk.mVoxels, sizeof(Voxel), VOXELS_PER_CHUNK, chunkFile);
	fclose(chunkFile);

	if (readCount != VOXELS_PER_CHUNK) {
		std::cerr << "Could only read " << readCount << " of " << VOXELS_PER_CHUNK
		          << " voxels from file: " << filePath << std::endl;
		return false;
	}

	// Set empty row flags.
	for (size_t y = 0; y < CHUNK_SIZE; y++) {
		for (size_t z = 0; z < CHUNK_SIZE; z++) {
			chunk.setEmptyRowFlag(y, z);
			for (size_t x = 0; x < CHUNK_SIZE; x++) {
				if (chunk.getVoxel(y, z, x).type() != VoxelType::AIR) {
					chunk.clearEmptyRowFlag(y, z);
					continue;
				}
			}
		}
	}

	return true;
}

bool writeChunk(Chunk& chunk, int yOffset, int zOffset, int xOffset, const std::string& worldName)
{
	static const size_t VOXELS_PER_CHUNK = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;
	std::string dirPath = directoryPath(worldName);
	std::string filePath = filename(yOffset, zOffset, xOffset, worldName);

	// Make sure directory exists
	if (!exists(dirPath)) {
		if(!createDirectory(dirPath)) {
			std::cerr << "Couldn't create directory: \"" << dirPath << "\", can't write chunk: \""
			          << filePath << "\"" << std::endl;
			return false;
		}
	}

	std::FILE* chunkFile = fopen(filePath.c_str(), "wb");
	if (chunkFile == NULL) return false;

	size_t writeCount = fwrite(chunk.mVoxels, sizeof(Voxel), VOXELS_PER_CHUNK, chunkFile);
	fclose(chunkFile);

	if (writeCount != VOXELS_PER_CHUNK) {
		std::cerr << "Could only write " << writeCount << " of " << VOXELS_PER_CHUNK
		          << " voxels to file: " << filePath << std::endl;
		return false;
	}

	return true;
}

} // namespace vox