#include "rendering/WorldRenderer.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

} // anonymous namespace

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

WorldRenderer::WorldRenderer(const World& world, const Assets& assets) noexcept
:
	mWorld{world},
	mAssets{assets}
{
	
}

// Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void WorldRenderer::drawWorld(const Camera& cam, GLuint shaderProgram) noexcept
{
	mat4f transform = sfz::identityMatrix4<float>();

	for (size_t i = 0; i < mWorld.mNumChunks; i++) {
		if (!mWorld.chunkAvailable(i)) continue;
		const Chunk* chunkPtr = mWorld.chunkPtr(i);
		vec3i chunkOffsetInt = mWorld.chunkOffset(i)*(int)CHUNK_SIZE;
		vec3f chunkOffset{(float)chunkOffsetInt[0], (float)chunkOffsetInt[1], (float)chunkOffsetInt[2]};

		vec3i part8Itr = chunkPartIterateBegin();
		while (part8Itr != chunkPartIterateEnd()) {
			const ChunkPart8* chunkPart8 = chunkPtr->chunkPart8Ptr(part8Itr);
			vec3i part4Itr = chunkPartIterateBegin();
			while (part4Itr != chunkPartIterateEnd()) {
				const ChunkPart4* chunkPart4 = chunkPart8->chunkPart4Ptr(part4Itr);
				vec3i part2Itr = chunkPartIterateBegin();
				while (part2Itr != chunkPartIterateEnd()) {
					const ChunkPart2* chunkPart2 = chunkPart4->chunkPart2Ptr(part2Itr);
					vec3i voxelItr = chunkPartIterateBegin();
					while (voxelItr != chunkPartIterateEnd()) {
						
						Voxel v = chunkPart2->getVoxel(voxelItr);
						voxelItr = chunkPartIterateNext(voxelItr);

						if (v.type() == vox::VoxelType::AIR) continue;
						vec3i voxelOffsetInt = part8Itr*8 + part4Itr*4 + part2Itr*2 + voxelItr;
						vec3f voxelOffset{(float)voxelOffsetInt[0], (float)voxelOffsetInt[1], (float)voxelOffsetInt[2]};

						sfz::translation(transform, chunkOffset + voxelOffset);
						gl::setUniform(shaderProgram, "modelMatrix", transform);

						glBindTexture(GL_TEXTURE_2D, mAssets.getCubeFaceTexture(v));
						mCubeObj.render();

					}
					part2Itr = chunkPartIterateNext(part2Itr);
				}
				part4Itr = chunkPartIterateNext(part4Itr);
			}
			part8Itr = chunkPartIterateNext(part8Itr);
		}
	}

	/*const Chunk* chunkPtr;
	vec3i offset;
	vec3f offsetVec;
	mat4f transform = sfz::identityMatrix4<float>();
	bool fullChunk;

	for (size_t i = 0; i < mWorld.mNumChunks; i++) {
		chunkPtr = mWorld.chunkPtr(i);
		if (chunkPtr == nullptr) continue;
		//if (chunkPtr->isEmptyChunk()) continue;

		offset = mWorld.chunkOffset(chunkPtr);
		offsetVec = mWorld.positionFromChunkOffset(offset);
		//fullChunk = chunkPtr->isFullChunk();


		for (size_t y = 0; y < vox::CHUNK_SIZE; y++) {
			//if (chunkPtr->isEmptyLayer(y)) continue;
			for (size_t z = 0; z < vox::CHUNK_SIZE; z++) {
				//if (chunkPtr->isEmptyXRow(y, z)) continue;
				for (size_t x = 0; x < vox::CHUNK_SIZE; x++) {

					vox::Voxel v = chunkPtr->getVoxel(x, y, z);
					if (v.type() == vox::VoxelType::AIR) continue;

					sfz::translation(transform, offsetVec + sfz::vec3f{static_cast<float>(x),
					                                                   static_cast<float>(y),
					                                                   static_cast<float>(z)});
					gl::setUniform(shaderProgram, "modelMatrix", transform);

					glBindTexture(GL_TEXTURE_2D, mAssets.getCubeFaceTexture(v));
					mCubeObj.render();
				}
			}
		}
	}*/
}


} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>