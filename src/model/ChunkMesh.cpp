#include "model/ChunkMesh.hpp"

#include <new> // std::nothrow

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// Anonymous namespace
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

namespace {

const vec3f CUBE_VERTICES[] = {
	// x, y, z
	// Left
	vec3f{0.0f, 0.0f, 0.0f}, // 0, left-bottom-back
	vec3f{0.0f, 0.0f, 1.0f}, // 1, left-bottom-front
	vec3f{0.0f, 1.0f, 0.0f}, // 2, left-top-back
	vec3f{0.0f, 1.0f, 1.0f}, // 3, left-top-front

	// Right
	vec3f{1.0f, 0.0f, 0.0f}, // 4, right-bottom-back
	vec3f{1.0f, 0.0f, 1.0f}, // 5, right-bottom-front
	vec3f{1.0f, 1.0f, 0.0f}, // 6, right-top-back
	vec3f{1.0f, 1.0f, 1.0f}, // 7, right-top-front

	// Bottom
	vec3f{0.0f, 0.0f, 0.0f}, // 8, left-bottom-back
	vec3f{0.0f, 0.0f, 1.0f}, // 9, left-bottom-front
	vec3f{1.0f, 0.0f, 0.0f}, // 10, right-bottom-back
	vec3f{1.0f, 0.0f, 1.0f}, // 11, right-bottom-front

	// Top
	vec3f{0.0f, 1.0f, 0.0f}, // 12, left-top-back
	vec3f{0.0f, 1.0f, 1.0f}, // 13, left-top-front
	vec3f{1.0f, 1.0f, 0.0f}, // 14, right-top-back
	vec3f{1.0f, 1.0f, 1.0f}, // 15, right-top-front

	// Back
	vec3f{0.0f, 0.0f, 0.0f}, // 16, left-bottom-back
	vec3f{0.0f, 1.0f, 0.0f}, // 17, left-top-back
	vec3f{1.0f, 0.0f, 0.0f}, // 18, right-bottom-back
	vec3f{1.0f, 1.0f, 0.0f}, // 19, right-top-back

	// Front
	vec3f{0.0f, 0.0f, 1.0f}, // 20, left-bottom-front
	vec3f{0.0f, 1.0f, 1.0f}, // 21, left-top-front
	vec3f{1.0f, 0.0f, 1.0f}, // 22, right-bottom-front
	vec3f{1.0f, 1.0f, 1.0f}  // 23, right-top-front
};

const vec3f CUBE_NORMALS[] = {
	// x, y, z
	// Left
	vec3f{-1.0f, 0.0f, 0.0f}, // 0, left-bottom-back
	vec3f{-1.0f, 0.0f, 0.0f}, // 1, left-bottom-front
	vec3f{-1.0f, 0.0f, 0.0f}, // 2, left-top-back
	vec3f{-1.0f, 0.0f, 0.0f}, // 3, left-top-front

	// Right
	vec3f{1.0f, 0.0f, 0.0f}, // 4, right-bottom-back
	vec3f{1.0f, 0.0f, 0.0f}, // 5, right-bottom-front
	vec3f{1.0f, 0.0f, 0.0f}, // 6, right-top-back
	vec3f{1.0f, 0.0f, 0.0f}, // 7, right-top-front

	// Bottom
	vec3f{0.0f, -1.0f, 0.0f}, // 8, left-bottom-back
	vec3f{0.0f, -1.0f, 0.0f}, // 9, left-bottom-front
	vec3f{0.0f, -1.0f, 0.0f}, // 10, right-bottom-back
	vec3f{0.0f, -1.0f, 0.0f}, // 11, right-bottom-front

	// Top
	vec3f{0.0f, 1.0f, 0.0f}, // 12, left-top-back
	vec3f{0.0f, 1.0f, 0.0f}, // 13, left-top-front
	vec3f{0.0f, 1.0f, 0.0f}, // 14, right-top-back
	vec3f{0.0f, 1.0f, 0.0f}, // 15, right-top-front

	// Back
	vec3f{0.0f, 0.0f, -1.0f}, // 16, left-bottom-back
	vec3f{0.0f, 0.0f, -1.0f}, // 17, left-top-back
	vec3f{0.0f, 0.0f, -1.0f}, // 18, right-bottom-back
	vec3f{0.0f, 0.0f, -1.0f}, // 19, right-top-back

	// Front
	vec3f{0.0f, 0.0f, 1.0f}, // 20, left-bottom-front
	vec3f{0.0f, 0.0f, 1.0f}, // 21, left-top-front
	vec3f{0.0f, 0.0f, 1.0f}, // 22, right-bottom-front
	vec3f{0.0f, 0.0f, 1.0f}  // 23, right-top-front
};

const vec2f CUBE_UV_COORDS[] = {
	// u, v
	// Left
	vec2f{0.0f, 0.0f}, // 0, left-bottom-back
	vec2f{1.0f, 0.0f}, // 1, left-bottom-front
	vec2f{0.0f, 1.0f}, // 2, left-top-back
	vec2f{1.0f, 1.0f}, // 3, left-top-front

	// Right
	vec2f{1.0f, 0.0f}, // 4, right-bottom-back
	vec2f{0.0f, 0.0f}, // 5, right-bottom-front
	vec2f{1.0f, 1.0f}, // 6, right-top-back
	vec2f{0.0f, 1.0f}, // 7, right-top-front

	// Bottom
	vec2f{0.0f, 0.0f}, // 8, left-bottom-back
	vec2f{0.0f, 1.0f}, // 9, left-bottom-front
	vec2f{1.0f, 0.0f}, // 10, right-bottom-back
	vec2f{1.0f, 1.0f}, // 11, right-bottom-front

	// Top
	vec2f{0.0f, 1.0f}, // 12, left-top-back
	vec2f{0.0f, 0.0f}, // 13, left-top-front
	vec2f{1.0f, 1.0f}, // 14, right-top-back
	vec2f{1.0f, 0.0f}, // 15, right-top-front

	// Back
	vec2f{1.0f, 0.0f}, // 16, left-bottom-back
	vec2f{1.0f, 1.0f}, // 17, left-top-back
	vec2f{0.0f, 0.0f}, // 18, right-bottom-back
	vec2f{0.0f, 1.0f}, // 19, right-top-back

	// Front
	vec2f{0.0f, 0.0f}, // 20, left-bottom-front
	vec2f{0.0f, 1.0f}, // 21, left-top-front
	vec2f{1.0f, 0.0f}, // 22, right-bottom-front
	vec2f{1.0f, 1.0f}  // 23, right-top-front
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

const size_t NUM_ELEMENTS = sizeof(CUBE_VERTICES)/sizeof(vec3f);
const size_t NUM_INDICES = sizeof(CUBE_INDICES)/sizeof(unsigned int);

template<typename T, size_t NUM_VOXELS, size_t NUM_DATA_ELEMENTS>
void fillArray(const unique_ptr<T[]>& array, const T* data)
{
	for (size_t iVox = 0; iVox < NUM_VOXELS; ++iVox) {
		for (size_t iArr = 0; iArr < NUM_DATA_ELEMENTS; ++iArr) {
			array[iVox + iArr] = data[iArr];
		}
	}
}

void addVoxelVertex(const unique_ptr<vec3f[]>& array, size_t startPos,
					const vec3f& position) noexcept
{
	for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
		array[startPos + i] = position + CUBE_VERTICES[i];
	}
}

void addVoxelUV(const unique_ptr<vec2f[]>& array, size_t startPos,
                const sfz::TextureRegion& texRegion) noexcept
{
	vec2f dim = texRegion.dimensions();
	for (size_t i = 0; i < NUM_ELEMENTS; ++i) {
		array[startPos + i] = texRegion.mUVMin + CUBE_UV_COORDS[i].elemMult(dim);
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
	mVertexArray{new (std::nothrow) vec3f[mDataArraySize]},
	mNormalArray{new (std::nothrow) vec3f[mDataArraySize]},
	mUVArray{new (std::nothrow) vec2f[mDataArraySize]},
	mIndexArray{new (std::nothrow) unsigned int[mIndicesArraySize]}
{
	static_assert(sizeof(vec2f) == sizeof(float)*2, "vec2f is padded");
	static_assert(sizeof(vec3f) == sizeof(float)*3, "vec3f is padded");
	static_assert(NUM_ELEMENTS == 24, "NUM_ELEMENTS is wrong size");
	static_assert(NUM_INDICES == 36, "NUM_INDICES is wrong size");

	const size_t NUM_VOXELS = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;
	fillArray<vec3f, NUM_VOXELS, NUM_ELEMENTS>(mNormalArray, CUBE_NORMALS);
	fillArray<unsigned int, NUM_VOXELS, NUM_INDICES>(mIndexArray, CUBE_INDICES);

	// Vertex buffer
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), NULL, GL_DYNAMIC_DRAW);

	// Normal buffer
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_NORMALS), &mNormalArray[0], GL_STATIC_DRAW);

	// UV buffer
	glGenBuffers(1, &mUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_UV_COORDS), NULL, GL_DYNAMIC_DRAW);

	// Index buffer
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mIndexBuffer), &mIndexArray[0], GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors likely caused by ChunkMesh constructor." << std::endl;
	}
}

ChunkMesh::~ChunkMesh() noexcept
{
	glDeleteBuffers(1, &mVertexBuffer);
	glDeleteBuffers(1, &mUVBuffer);
	glDeleteBuffers(1, &mNormalBuffer);
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
		index++;
		if (v.type() == VoxelType::AIR) continue;

		addVoxelVertex(mVertexArray, mCurrentNumVoxels, index.voxelOffset());
		addVoxelUV(mUVArray, mCurrentNumVoxels, sfz::TextureRegion{vec2f{0.0f, 0.0f}, vec2f{1.0f, 1.0f}});

		mCurrentNumVoxels++;
	}

	// Transfer data to GPU.
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES)*mNumVoxelsPerChunk, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CUBE_VERTICES)*mCurrentNumVoxels, mVertexArray[0].glPtr());

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_UV_COORDS)*mNumVoxelsPerChunk, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CUBE_UV_COORDS)*mCurrentNumVoxels, mUVArray[0].glPtr());

	// Rebind VAO parameters just in case
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

void ChunkMesh::render() noexcept
{
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElements(GL_TRIANGLES, NUM_INDICES*mNumVoxelsPerChunk, GL_UNSIGNED_INT, NULL);
}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>