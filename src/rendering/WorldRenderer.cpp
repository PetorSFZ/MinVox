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
	AABB aabb;

	for (size_t i = 0; i < mWorld.mNumChunks; i++) {
		if (!mWorld.chunkAvailable(i)) continue;
		const Chunk* chunkPtr = mWorld.chunkPtr(i);

		vec3i offset = mWorld.chunkOffset(i);
		vec3f offsetVec = mWorld.positionFromChunkOffset(offset);

		calculateChunkAABB(aabb, offsetVec);
		if (!cam.isVisible(aabb)) continue;

		ChunkIndex index = ChunkIterateBegin;
		for (unsigned int part8i = 0; part8i < 8; part8i++) {
			calculateChunkPart8AABB(aabb, offsetVec, index);
			if (!cam.isVisible(aabb)) {
				index.plusPart8();
				continue;
			}
			for (unsigned int part4i = 0; part4i < 8; part4i++) {
				calculateChunkPart4AABB(aabb, offsetVec, index);
				if (!cam.isVisible(aabb)) {
					index.plusPart4();
					continue;
				}
				for (unsigned int voxeli = 0; voxeli < 64; voxeli++) {

					Voxel v = chunkPtr->getVoxel(index);
					if (v.type() == vox::VoxelType::AIR) {
						index++;
						continue;
					}

					sfz::translation(transform, offsetVec + index.voxelOffset());
					gl::setUniform(shaderProgram, "modelMatrix", transform);

					glBindTexture(GL_TEXTURE_2D, mAssets.getCubeFaceTexture(v));
					mCubeObj.render();

					index++;
				}
			}
		}
		sfz_assert_debug(index == ChunkIterateEnd);
	}
}

void WorldRenderer::drawWorldOld(const Camera& cam, GLuint shaderProgram) noexcept
{
	// Old naive loop
	mat4f transform = sfz::identityMatrix4<float>();
	AABB aabb;

	for (size_t i = 0; i < mWorld.mNumChunks; i++) {
		if (!mWorld.chunkAvailable(i)) continue;
		const Chunk* chunkPtr = mWorld.chunkPtr(i);

		vec3i offset = mWorld.chunkOffset(i);
		vec3f offsetVec = mWorld.positionFromChunkOffset(offset);

		calculateChunkAABB(aabb, offset);
		if (!cam.isVisible(aabb)) continue;

		for (size_t x = 0; x < CHUNK_SIZE; x++) {
			for (size_t y = 0; y < CHUNK_SIZE; y++) {
				for (size_t z = 0; z < CHUNK_SIZE; z++) {

					Voxel v = chunkPtr->getVoxel(x, y, z);
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
	}
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>