#include "rendering/FullscreenQuadObject.hpp"

#include <iostream>

namespace vox {

FullscreenQuadObject::FullscreenQuadObject()
{
	const float positions[] = {
		-1.0f, -1.0f, // bottom-left
		1.0f, -1.0f, // bottom-right
		-1.0f, 1.0f, // top-left
		1.0f, 1.0f // top-right
	};
	const unsigned int indices[] = {
		0, 1, 2,
		1, 3, 2
	};
	const float uvCoords[] = {
		// bottom-left UV
		0.0f, 0.0f,
		// bottom-right UV
		1.0f, 0.0f,
		// top-left UV
		0.0f, 1.0f,
		// top-right UV
		1.0f, 1.0f
	};

	// Buffer objects
	glGenBuffers(1, &mPosBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &mUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoords), uvCoords, GL_STATIC_DRAW);

	// Vertex Array Object
	glGenVertexArrays(1, &mVertexArrayObject);
	glBindVertexArray(mVertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, mPosBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, mUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	if (gl::checkAllGLErrors()) {
		std::cerr << "^^^ Above errors likely caused by FullscreenQuadObject ctor." << std::endl;
	}
}

FullscreenQuadObject::~FullscreenQuadObject()
{
	glDeleteBuffers(1, &mPosBuffer);
	glDeleteBuffers(1, &mUVBuffer);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteVertexArrays(1, &mVertexArrayObject);
}

void FullscreenQuadObject::render()
{
	glBindVertexArray(mVertexArrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

} // namespace vox