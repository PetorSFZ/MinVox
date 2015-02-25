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

		// Right
		1.0f, 0.0f, 0.0f, // 8, right-bottom-back
		1.0f, 0.0f, 1.0f, // 9, right-bottom-front
		1.0f, 1.0f, 0.0f, // 10, right-top-back
		1.0f, 1.0f, 1.0f, // 11, right-top-front

		// Left
		0.0f, 0.0f, 0.0f, // 12, left-bottom-back
		0.0f, 0.0f, 1.0f, // 13, left-bottom-front
		0.0f, 1.0f, 0.0f, // 14, left-top-back
		0.0f, 1.0f, 1.0f, // 15, left-top-front

		// Back
		0.0f, 0.0f, 0.0f, // 16, back-left-bottom
		1.0f, 0.0f, 0.0f, // 17, back-right-bottom
		0.0f, 1.0f, 0.0f, // 18, back-left-top
		1.0f, 1.0f, 0.0f, // 19, back-right-top

		// Front
		0.0f, 0.0f, 1.0f, // 20, front-left-bottom
		1.0f, 0.0f, 1.0f, // 21, front-right-bottom
		0.0f, 1.0f, 1.0f, // 22, front-left-top
		1.0f, 1.0f, 1.0f  // 23, front-right-top
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

		// Right
		0.0f, 0.0f, // 8, right-bottom-back
		0.0f, 1.0f, // 9, right-bottom-front
		1.0f, 0.0f, // 10, right-top-back
		1.0f, 1.0f, // 11, right-top-front

		// Left
		0.0f, 0.0f, // 12, left-bottom-back
		0.0f, 1.0f, // 13, left-bottom-front
		1.0f, 0.0f, // 14, left-top-back
		1.0f, 1.0f, // 15, left-top-front

		// Back
		0.0f, 0.0f, // 16, back-left-bottom
		0.0f, 1.0f, // 17, back-right-bottom
		1.0f, 0.0f, // 18, back-left-top
		1.0f, 1.0f, // 19, back-right-top

		// Front
		0.0f, 0.0f, // 20, front-left-bottom
		0.0f, 1.0f, // 21, front-right-bottom
		1.0f, 0.0f, // 22, front-left-top
		1.0f, 1.0f  // 23, front-right-top
	};
	const float normals[] = {
		// Bottom
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		// Top
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Left
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		// Back
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		// Front
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};
	const unsigned int indices[] = {
		// Bottom
		0, 1, 2,
		2, 1, 3,

		// Top
		6, 5, 4,
		7, 5, 6,

		// Right
		10, 9, 8,
		11, 9, 10,

		// Left
		12, 13, 14,
		14, 13, 15,

		// Back
		18, 17, 16,
		19, 17, 18,

		// Front
		20, 21, 22,
		22, 21, 23
	};

	// Buffer objects
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoords), uvCoords, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// Vertex Array Object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(2);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors likely caused by CubeObject constructor." << std::endl;
	}
}

CubeObject::~CubeObject()
{
	glDeleteBuffers(1, &posBuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &vertexArrayObject);
}

// Public functions
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *


void CubeObject::render()
{
	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL);
}

} // namespace vox