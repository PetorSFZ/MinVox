#include "model/ChunkMesh.hpp"

#include <new> // std::nothrow
#include "sfz/GL.hpp"
#include "rendering/Assets.hpp"



namespace vox {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

const vec3 CUBE_VERTICES[] = {
	// x, y, z
	// Left
	vec3{0.0f, 0.0f, 0.0f}, // 0, left-bottom-back
	vec3{0.0f, 0.0f, 1.0f}, // 1, left-bottom-front
	vec3{0.0f, 1.0f, 0.0f}, // 2, left-top-back
	vec3{0.0f, 1.0f, 1.0f}, // 3, left-top-front

	// Right
	vec3{1.0f, 0.0f, 0.0f}, // 4, right-bottom-back
	vec3{1.0f, 0.0f, 1.0f}, // 5, right-bottom-front
	vec3{1.0f, 1.0f, 0.0f}, // 6, right-top-back
	vec3{1.0f, 1.0f, 1.0f}, // 7, right-top-front

	// Bottom
	vec3{0.0f, 0.0f, 0.0f}, // 8, left-bottom-back
	vec3{0.0f, 0.0f, 1.0f}, // 9, left-bottom-front
	vec3{1.0f, 0.0f, 0.0f}, // 10, right-bottom-back
	vec3{1.0f, 0.0f, 1.0f}, // 11, right-bottom-front

	// Top
	vec3{0.0f, 1.0f, 0.0f}, // 12, left-top-back
	vec3{0.0f, 1.0f, 1.0f}, // 13, left-top-front
	vec3{1.0f, 1.0f, 0.0f}, // 14, right-top-back
	vec3{1.0f, 1.0f, 1.0f}, // 15, right-top-front

	// Back
	vec3{0.0f, 0.0f, 0.0f}, // 16, left-bottom-back
	vec3{0.0f, 1.0f, 0.0f}, // 17, left-top-back
	vec3{1.0f, 0.0f, 0.0f}, // 18, right-bottom-back
	vec3{1.0f, 1.0f, 0.0f}, // 19, right-top-back

	// Front
	vec3{0.0f, 0.0f, 1.0f}, // 20, left-bottom-front
	vec3{0.0f, 1.0f, 1.0f}, // 21, left-top-front
	vec3{1.0f, 0.0f, 1.0f}, // 22, right-bottom-front
	vec3{1.0f, 1.0f, 1.0f}  // 23, right-top-front
};

const vec3 CUBE_NORMALS[] = {
	// x, y, z
	// Left
	vec3{-1.0f, 0.0f, 0.0f}, // 0, left-bottom-back
	vec3{-1.0f, 0.0f, 0.0f}, // 1, left-bottom-front
	vec3{-1.0f, 0.0f, 0.0f}, // 2, left-top-back
	vec3{-1.0f, 0.0f, 0.0f}, // 3, left-top-front

	// Right
	vec3{1.0f, 0.0f, 0.0f}, // 4, right-bottom-back
	vec3{1.0f, 0.0f, 0.0f}, // 5, right-bottom-front
	vec3{1.0f, 0.0f, 0.0f}, // 6, right-top-back
	vec3{1.0f, 0.0f, 0.0f}, // 7, right-top-front

	// Bottom
	vec3{0.0f, -1.0f, 0.0f}, // 8, left-bottom-back
	vec3{0.0f, -1.0f, 0.0f}, // 9, left-bottom-front
	vec3{0.0f, -1.0f, 0.0f}, // 10, right-bottom-back
	vec3{0.0f, -1.0f, 0.0f}, // 11, right-bottom-front

	// Top
	vec3{0.0f, 1.0f, 0.0f}, // 12, left-top-back
	vec3{0.0f, 1.0f, 0.0f}, // 13, left-top-front
	vec3{0.0f, 1.0f, 0.0f}, // 14, right-top-back
	vec3{0.0f, 1.0f, 0.0f}, // 15, right-top-front

	// Back
	vec3{0.0f, 0.0f, -1.0f}, // 16, left-bottom-back
	vec3{0.0f, 0.0f, -1.0f}, // 17, left-top-back
	vec3{0.0f, 0.0f, -1.0f}, // 18, right-bottom-back
	vec3{0.0f, 0.0f, -1.0f}, // 19, right-top-back

	// Front
	vec3{0.0f, 0.0f, 1.0f}, // 20, left-bottom-front
	vec3{0.0f, 0.0f, 1.0f}, // 21, left-top-front
	vec3{0.0f, 0.0f, 1.0f}, // 22, right-bottom-front
	vec3{0.0f, 0.0f, 1.0f}  // 23, right-top-front
};

const vec2 CUBE_UV_COORDS[] = {
	// u, v
	// Left
	vec2{0.0f, 0.0f}, // 0, left-bottom-back
	vec2{1.0f, 0.0f}, // 1, left-bottom-front
	vec2{0.0f, 1.0f}, // 2, left-top-back
	vec2{1.0f, 1.0f}, // 3, left-top-front

	// Right
	vec2{1.0f, 0.0f}, // 4, right-bottom-back
	vec2{0.0f, 0.0f}, // 5, right-bottom-front
	vec2{1.0f, 1.0f}, // 6, right-top-back
	vec2{0.0f, 1.0f}, // 7, right-top-front

	// Bottom
	vec2{0.0f, 0.0f}, // 8, left-bottom-back
	vec2{0.0f, 1.0f}, // 9, left-bottom-front
	vec2{1.0f, 0.0f}, // 10, right-bottom-back
	vec2{1.0f, 1.0f}, // 11, right-bottom-front

	// Top
	vec2{0.0f, 1.0f}, // 12, left-top-back
	vec2{0.0f, 0.0f}, // 13, left-top-front
	vec2{1.0f, 1.0f}, // 14, right-top-back
	vec2{1.0f, 0.0f}, // 15, right-top-front

	// Back
	vec2{1.0f, 0.0f}, // 16, left-bottom-back
	vec2{1.0f, 1.0f}, // 17, left-top-back
	vec2{0.0f, 0.0f}, // 18, right-bottom-back
	vec2{0.0f, 1.0f}, // 19, right-top-back

	// Front
	vec2{0.0f, 0.0f}, // 20, left-bottom-front
	vec2{0.0f, 1.0f}, // 21, left-top-front
	vec2{1.0f, 0.0f}, // 22, right-bottom-front
	vec2{1.0f, 1.0f}  // 23, right-top-front
};

const unsigned int CUBE_INDICES[] = {
	// Left
	0, 1, 2,
	3, 2, 1,

	// Right
	5, 4, 7,
	6, 7, 4,

	// Bottom
	8, 10, 9,
	11, 9, 10,

	// Top
	13, 15, 12,
	14, 12, 15,

	// Back
	18, 16, 19,
	17, 19, 16,

	// Front
	20, 22, 21,
	23, 21, 22
};

const size_t NUM_ELEMENTS = sizeof(CUBE_VERTICES)/sizeof(vec3);
const size_t NUM_INDICES = sizeof(CUBE_INDICES)/sizeof(unsigned int);

void fillNormalArray(const unique_ptr<vec3[]>& array, size_t numVoxelsPerChunk) noexcept
{
	for (size_t iVox = 0; iVox < numVoxelsPerChunk; ++iVox) {
		size_t arrayPos = iVox * NUM_ELEMENTS;
		for (size_t iArr = 0; iArr < NUM_ELEMENTS; ++iArr) {
			array[arrayPos + iArr] = CUBE_NORMALS[iArr];
		}
	}
}

void fillIndexArray(const unique_ptr<unsigned int[]>& array, size_t numVoxelsPerChunk) noexcept
{
	for (size_t iVox = 0; iVox < numVoxelsPerChunk; ++iVox) {
		size_t arrayPos = iVox * NUM_INDICES;
		unsigned int indexOffset = iVox * NUM_ELEMENTS;
		for (size_t iArr = 0; iArr < NUM_INDICES; ++iArr) {
			array[arrayPos + iArr] = indexOffset + CUBE_INDICES[iArr];
		}
	}
}

void addVoxelVertex(const unique_ptr<vec3[]>& array, size_t voxelNum,
					const vec3& position) noexcept
{
	size_t arrayPos = voxelNum * NUM_ELEMENTS;
	for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
		array[arrayPos + i] = position + CUBE_VERTICES[i];
	}
}

void addVoxelUV(const unique_ptr<vec2[]>& array, size_t voxelNum,
                const gl::TextureRegion& texRegion) noexcept
{
	size_t arrayPos = voxelNum * NUM_ELEMENTS;
	vec2 dim = texRegion.dimensions();
	for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
		array[arrayPos + i] = texRegion.mUVMin + (CUBE_UV_COORDS[i] * dim);
	}
}

} // anonymous namespace

// ChunkMesh: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ChunkMesh::ChunkMesh() noexcept
:
	mNumVoxelsPerChunk{CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE},
	mDataArraySize{NUM_ELEMENTS * mNumVoxelsPerChunk},
	mIndicesArraySize{NUM_INDICES * mNumVoxelsPerChunk},
	mVertexArray{new (std::nothrow) vec3[mDataArraySize]},
	mNormalArray{new (std::nothrow) vec3[mDataArraySize]},
	mUVArray{new (std::nothrow) vec2[mDataArraySize]},
	mIndexArray{new (std::nothrow) unsigned int[mIndicesArraySize]}
{
	static_assert(sizeof(vec2) == sizeof(float)*2, "vec2 is padded");
	static_assert(sizeof(vec3) == sizeof(float)*3, "vec3 is padded");
	static_assert(NUM_ELEMENTS == 24, "NUM_ELEMENTS is wrong size");
	static_assert(NUM_INDICES == 36, "NUM_INDICES is wrong size");

	fillNormalArray(mNormalArray, mNumVoxelsPerChunk);
	fillIndexArray(mIndexArray, mNumVoxelsPerChunk);

	// Vertex buffer
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES)*mNumVoxelsPerChunk, NULL, GL_DYNAMIC_DRAW);

	// Normal buffer
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_NORMALS)*mNumVoxelsPerChunk, mNormalArray[0].elements, GL_STATIC_DRAW);

	// UV buffer
	glGenBuffers(1, &mUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_UV_COORDS)*mNumVoxelsPerChunk, NULL, GL_DYNAMIC_DRAW);

	// Index buffer
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_INDICES)*mNumVoxelsPerChunk, &mIndexArray[0], GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

ChunkMesh::~ChunkMesh() noexcept
{
	glDeleteBuffers(1, &mVertexBuffer);
	glDeleteBuffers(1, &mNormalBuffer);
	glDeleteBuffers(1, &mUVBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVAO);
}

// ChunkMesh: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void ChunkMesh::set(const Chunk& chunk) noexcept
{
	mCurrentNumVoxels = 0;
	ChunkIndex index = ChunkIterateBegin;

	while (index != ChunkIterateEnd) {
		Voxel v = chunk.getVoxel(index);

		if (v.mType == VOXEL_AIR) {
			index++;
			continue;
		}

		addVoxelVertex(mVertexArray, mCurrentNumVoxels, index.voxelOffset());
		addVoxelUV(mUVArray, mCurrentNumVoxels, Assets::INSTANCE().cubeFaceRegion(v));
		mCurrentNumVoxels += 1;
		index++;
	}

	// Transfer data to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES)*mNumVoxelsPerChunk, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CUBE_VERTICES)*mCurrentNumVoxels, mVertexArray[0].elements);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_UV_COORDS)*mNumVoxelsPerChunk, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CUBE_UV_COORDS)*mCurrentNumVoxels, mUVArray[0].elements);

	// Rebind VAO parameters just in case
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

void ChunkMesh::render() const noexcept
{
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElements(GL_TRIANGLES, NUM_INDICES*mCurrentNumVoxels, GL_UNSIGNED_INT, NULL);
}

} // namespace vox

