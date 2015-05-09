#include "model/ChunkModel.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// ChunkModel: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

ChunkModel::ChunkModel() noexcept
{
	static_assert(sizeof(vec3f) == sizeof(float)*3, "vec3f is padded");

	// Vertex buffer
	const float vertices[] = {
		// x, y, z
		// Left
		0.0f, 0.0f, 0.0f, // 0, left-bottom-back
		0.0f, 0.0f, 1.0f, // 1, left-bottom-front
		0.0f, 1.0f, 0.0f, // 2, left-top-back
		0.0f, 1.0f, 1.0f, // 3, left-top-front

		// Right
		1.0f, 0.0f, 0.0f, // 4, right-bottom-back
		1.0f, 0.0f, 1.0f, // 5, right-bottom-front
		1.0f, 1.0f, 0.0f, // 6, right-top-back
		1.0f, 1.0f, 1.0f, // 7, right-top-front

		// Bottom
		0.0f, 0.0f, 0.0f, // 8, left-bottom-back
		0.0f, 0.0f, 1.0f, // 9, left-bottom-front
		1.0f, 0.0f, 0.0f, // 10, right-bottom-back
		1.0f, 0.0f, 1.0f, // 11, right-bottom-front

		// Top
		0.0f, 1.0f, 0.0f, // 12, left-top-back
		0.0f, 1.0f, 1.0f, // 13, left-top-front
		1.0f, 1.0f, 0.0f, // 14, right-top-back
		1.0f, 1.0f, 1.0f, // 15, right-top-front

		// Back
		0.0f, 0.0f, 0.0f, // 16, left-bottom-back
		0.0f, 1.0f, 0.0f, // 17, left-top-back
		1.0f, 0.0f, 0.0f, // 18, right-bottom-back
		1.0f, 1.0f, 0.0f, // 19, right-top-back

		// Front
		0.0f, 0.0f, 1.0f, // 20, left-bottom-front
		0.0f, 1.0f, 1.0f, // 21, left-top-front
		1.0f, 0.0f, 1.0f, // 22, right-bottom-front
		1.0f, 1.0f, 1.0f  // 23, right-top-front
	};
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Normal buffer
	const float normals[] = {
		// x, y, z
		// Left
		-1.0f, 0.0f, 0.0f, // 0, left-bottom-back
		-1.0f, 0.0f, 0.0f, // 1, left-bottom-front
		-1.0f, 0.0f, 0.0f, // 2, left-top-back
		-1.0f, 0.0f, 0.0f, // 3, left-top-front

		// Right
		1.0f, 0.0f, 0.0f, // 4, right-bottom-back
		1.0f, 0.0f, 0.0f, // 5, right-bottom-front
		1.0f, 0.0f, 0.0f, // 6, right-top-back
		1.0f, 0.0f, 0.0f, // 7, right-top-front

		// Bottom
		0.0f, -1.0f, 0.0f, // 8, left-bottom-back
		0.0f, -1.0f, 0.0f, // 9, left-bottom-front
		0.0f, -1.0f, 0.0f, // 10, right-bottom-back
		0.0f, -1.0f, 0.0f, // 11, right-bottom-front

		// Top
		0.0f, 1.0f, 0.0f, // 12, left-top-back
		0.0f, 1.0f, 0.0f, // 13, left-top-front
		0.0f, 1.0f, 0.0f, // 14, right-top-back
		0.0f, 1.0f, 0.0f, // 15, right-top-front

		// Back
		0.0f, 0.0f, -1.0f, // 16, left-bottom-back
		0.0f, 0.0f, -1.0f, // 17, left-top-back
		0.0f, 0.0f, -1.0f, // 18, right-bottom-back
		0.0f, 0.0f, -1.0f, // 19, right-top-back

		// Front
		0.0f, 0.0f, 1.0f, // 20, left-bottom-front
		0.0f, 0.0f, 1.0f, // 21, left-top-front
		0.0f, 0.0f, 1.0f, // 22, right-bottom-front
		0.0f, 0.0f, 1.0f  // 23, right-top-front
	};
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	// UV buffer
	const float uvCoords[] = {
		// u, v
		// Left
		0.0f, 0.0f, // 0, left-bottom-back
		1.0f, 0.0f, // 1, left-bottom-front
		0.0f, 1.0f, // 2, left-top-back
		1.0f, 1.0f, // 3, left-top-front

		// Right
		1.0f, 0.0f, // 4, right-bottom-back
		0.0f, 0.0f, // 5, right-bottom-front
		1.0f, 1.0f, // 6, right-top-back
		0.0f, 1.0f, // 7, right-top-front

		// Bottom
		0.0f, 0.0f, // 8, left-bottom-back
		0.0f, 1.0f, // 9, left-bottom-front
		1.0f, 0.0f, // 10, right-bottom-back
		1.0f, 1.0f, // 11, right-bottom-front

		// Top
		0.0f, 1.0f, // 12, left-top-back
		0.0f, 0.0f, // 13, left-top-front
		1.0f, 1.0f, // 14, right-top-back
		1.0f, 0.0f, // 15, right-top-front

		// Back
		1.0f, 0.0f, // 16, left-bottom-back
		1.0f, 1.0f, // 17, left-top-back
		0.0f, 0.0f, // 18, right-bottom-back
		0.0f, 1.0f, // 19, right-top-back

		// Front
		0.0f, 0.0f, // 20, left-bottom-front
		0.0f, 1.0f, // 21, left-top-front
		1.0f, 0.0f, // 22, right-bottom-front
		1.0f, 1.0f  // 23, right-top-front
	};
	glGenBuffers(1, &mUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoords), uvCoords, GL_STATIC_DRAW);

	// Index buffer
	const unsigned int indices[] = {
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
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	const size_t CHUNK_SIZE_3 = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

	// Position buffer (null now, to be updated later)
	glGenBuffers(1, &mPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3f)*CHUNK_SIZE_3, NULL, GL_STREAM_DRAW);

	// Texture ID buffer (null now, to be updated later)
	glGenBuffers(1, &mTexIDBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTexIDBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(int)*CHUNK_SIZE_3, NULL, GL_STREAM_DRAW);



	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors likely caused by ChunkModel constructor." << std::endl;
	}
}

ChunkModel::~ChunkModel() noexcept
{

}

// ChunkModel: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void ChunkModel::set(const Chunk& chunk) noexcept
{

}

void ChunkModel::render() noexcept
{

}

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>