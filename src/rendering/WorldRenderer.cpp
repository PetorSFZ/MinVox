#include "rendering/WorldRenderer.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

vec3f offsetToVector(const vec3i& offset) noexcept
{
	return vec3f{(float)offset[0], (float)offset[1], (float)offset[2]};
}

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

	/*mat4f transform = sfz::identityMatrix4<float>();
	AABB aabb;

	for (size_t i = 0; i < mWorld.mNumChunks; i++) {
		if (!mWorld.chunkAvailable(i)) continue;
		const Chunk* chunkPtr = mWorld.chunkPtr(i);
		vec3i chunkOffset = mWorld.chunkOffset(i);

		calculateChunkAABB(aabb, chunkOffset);
		if (!cam.isVisible(aabb)) continue;

		vec3i part8Itr = chunkPartIterateBegin();
		while (part8Itr != chunkPartIterateEnd()) {
			calculateChunkPart8AABB(aabb, chunkOffset, part8Itr);
			if (!cam.isVisible(aabb)) {
				part8Itr = chunkPartIterateNext(part8Itr);
				continue;
			}
			const ChunkPart8* chunkPart8 = chunkPtr->chunkPart8Ptr(part8Itr);
			
			vec3i part4Itr = chunkPartIterateBegin();
			while (part4Itr != chunkPartIterateEnd()) {
				calculateChunkPart4AABB(aabb, chunkOffset, part8Itr, part4Itr);
				if (!cam.isVisible(aabb)) {
					part4Itr = chunkPartIterateNext(part4Itr);
					continue;
				}
				const ChunkPart4* chunkPart4 = chunkPart8->chunkPart4Ptr(part4Itr);

				vec3i part2Itr = chunkPartIterateBegin();
				while (part2Itr != chunkPartIterateEnd()) {
					const ChunkPart2* chunkPart2 = chunkPart4->chunkPart2Ptr(part2Itr);
					
					vec3i voxelItr = chunkPartIterateBegin();
					while (voxelItr != chunkPartIterateEnd()) {
						Voxel v = chunkPart2->getVoxel(voxelItr);

						if (v.type() == vox::VoxelType::AIR) {
							voxelItr = chunkPartIterateNext(voxelItr);
							continue;
						}

						vec3f voxelPosition = offsetToVector(chunkOffset*16 + part8Itr*8 + part4Itr*4 + part2Itr*2 + voxelItr);
						sfz::translation(transform, voxelPosition);
						gl::setUniform(shaderProgram, "modelMatrix", transform);

						glBindTexture(GL_TEXTURE_2D, mAssets.getCubeFaceTexture(v));
						mCubeObj.render();

						voxelItr = chunkPartIterateNext(voxelItr);
					}
					part2Itr = chunkPartIterateNext(part2Itr);
				}
				part4Itr = chunkPartIterateNext(part4Itr);
			}
			part8Itr = chunkPartIterateNext(part8Itr);
		}
	}*/
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