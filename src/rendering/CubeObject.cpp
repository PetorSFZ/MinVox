#include "rendering/CubeObject.hpp"

namespace vox {

// Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

CubeObject::CubeObject()
{
	const float positions[] = {
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
	const float flatNormals[] = {
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
	/*const float val = 1.0f / std::sqrtf(3.0f);
	const float sphereNormals[] = {
		// x, y, z
		// Left
		-val, -val, -val, // 0, left-bottom-back
		-val, -val, val, // 1, left-bottom-front
		-val, val, -val, // 2, left-top-back
		-val, val, val, // 3, left-top-front

		// Right
		val, -val, -val, // 4, right-bottom-back
		val, -val, val, // 5, right-bottom-front
		val, val, -val, // 6, right-top-back
		val, val, val, // 7, right-top-front

		// Bottom
		-val, -val, -val, // 8, left-bottom-back
		-val, -val, val, // 9, left-bottom-front
		val, -val, -val, // 10, right-bottom-back
		val, -val, val, // 11, right-bottom-front

		// Top
		-val, val, -val, // 12, left-top-back
		-val, val, val, // 13, left-top-front
		val, val, -val, // 14, right-top-back
		val, val, val, // 15, right-top-front

		// Back
		-val, -val, -val, // 16, left-bottom-back
		-val, val, -val, // 17, left-top-back
		val, -val, -val, // 18, right-bottom-back
		val, val, -val, // 19, right-top-back

		// Front
		-val, -val, val, // 20, left-bottom-front
		-val, val, val, // 21, left-top-front
		val, -val, val, // 22, right-bottom-front
		val, val, val, // 23, right-top-front
	};*/
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

	// Buffer objects
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(flatNormals), flatNormals, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(sphereNormals), sphereNormals, GL_STATIC_DRAW);

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