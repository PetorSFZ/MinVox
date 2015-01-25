#include "rendering/CubeObject.hpp"

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

CubeObject::CubeObject()
{
	const float positions[] = {
		// Bottom
		1.0f, 0.0f, 0.0f, // 0, bottom-bottom-left
		1.0f, 0.0f, 1.0f, // 1, bottom-bottom-right
		0.0f, 0.0f, 0.0f, // 2, bottom-top-left
		0.0f, 0.0f, 1.0f, // 3, bottom-top-right

		// Top
		1.0f, 1.0f, 0.0f, // 4, top-bottom-left
		1.0f, 1.0f, 1.0f, // 5, top-bottom-right
		0.0f, 1.0f, 0.0f, // 6, top-top-left
		0.0f, 1.0f, 1.0f, // 7, top-top-right

		// Front
		1.0f, 0.0f, 0.0f, // 8, front-bottom-left
		1.0f, 0.0f, 1.0f, // 9, front-bottom-right
		1.0f, 1.0f, 0.0f, // 10, front-top-left
		1.0f, 1.0f, 1.0f, // 11, front-top-right

		// Back
		0.0f, 0.0f, 0.0f, // 12, back-bottom-left
		0.0f, 0.0f, 1.0f, // 13, back-bottom-right
		0.0f, 1.0f, 0.0f, // 14, back-top-left
		0.0f, 1.0f, 1.0f, // 15, back-top-right

		// Left
		0.0f, 0.0f, 0.0f, // 16, left-bottom-left
		1.0f, 0.0f, 0.0f, // 17, left-bottom-right
		0.0f, 1.0f, 0.0f, // 18, left-top-left
		1.0f, 1.0f, 0.0f, // 19, left-top-right

		// Right
		0.0f, 0.0f, 1.0f, // 20, right-bottom-left
		1.0f, 0.0f, 1.0f, // 21, right-bottom-right
		0.0f, 1.0f, 1.0f, // 22, right-top-left
		1.0f, 1.0f, 1.0f  // 23, right-top-right
	};
	const float uvCoords[] = {
		// Bottom
		0.0f, 0.0f, // 0, bottom-bottom-left
		0.0f, 1.0f, // 1, bottom-bottom-right
		1.0f, 0.0f, // 2, bottom-top-left
		1.0f, 1.0f, // 3, bottom-top-right

		// Top
		0.0f, 0.0f, // 4, top-bottom-left
		0.0f, 1.0f, // 5, top-bottom-right
		1.0f, 0.0f, // 6, top-top-left
		1.0f, 1.0f, // 7, top-top-right

		// Front
		0.0f, 0.0f, // 8, front-bottom-left
		0.0f, 1.0f, // 9, front-bottom-right
		1.0f, 0.0f, // 10, front-top-left
		1.0f, 1.0f, // 11, front-top-right

		// Back
		0.0f, 0.0f, // 12, back-bottom-left
		0.0f, 1.0f, // 13, back-bottom-right
		1.0f, 0.0f, // 14, back-top-left
		1.0f, 1.0f, // 15, back-top-right

		// Left
		0.0f, 0.0f, // 16, left-bottom-left
		0.0f, 1.0f, // 17, left-bottom-right
		1.0f, 0.0f, // 18, left-top-left
		1.0f, 1.0f, // 19, left-top-right

		// Right
		0.0f, 0.0f, // 20, right-bottom-left
		0.0f, 1.0f, // 21, right-bottom-right
		1.0f, 0.0f, // 22, right-top-left
		1.0f, 1.0f  // 23, right-top-right
	};
	const unsigned int indices[] = {
		// Bottom
		0, 1, 2,
		2, 1, 3,

		// Top
		6, 5, 4,
		7, 5, 6,

		// Front
		10, 9, 8,
		11, 9, 10,

		// Back
		12, 13, 14,
		14, 13, 15,

		// Left
		18, 17, 16,
		19, 17, 18,

		// Right
		20, 21, 22,
		22, 21, 23
	};

	// Buffer objects
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoords), uvCoords, GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors likely caused by CubeObject constructor." << std::endl;
	}
}

CubeObject::~CubeObject()
{
	glDeleteBuffers(1, &posBuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vertexArrayObject);
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


void CubeObject::render()
{
	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
}

} // namespace vox