#include "sfz/gl/Utils.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace gl {

bool checkGLError() noexcept
{
	GLenum errorCode = glGetError();
	if (errorCode != GL_NO_ERROR) {
		std::cerr << "OpenGL error " << errorCode << ", " << gluErrorString(errorCode)
		          << std::endl;
		return true;
	}
	return false;
}

bool checkAllGLErrors() noexcept
{
	bool foundError = false;
	GLenum errorCode = glGetError();
	while (errorCode != GL_NO_ERROR) {
		std::cerr << "OpenGL error " << errorCode << ", " << gluErrorString(errorCode)
		          << std::endl;
		errorCode = glGetError();
		foundError = true;
	}
	return foundError;
}

void printShaderInfoLog(GLuint program) noexcept
{
	int logLength;
	glGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);
	char* log = new char[logLength+1];
	glGetShaderInfoLog(program, logLength, NULL, log);
	std::cerr << log << std::endl;
	delete[] log;
}

GLuint compileVertexShader(const std::string& shaderSource) noexcept
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* shaderSourcePtr = shaderSource.c_str();
	glShaderSource(vertexShader, 1, &shaderSourcePtr, NULL);
	glCompileShader(vertexShader);
	{
		int compileSuccess;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileSuccess);
		if (!compileSuccess) {
			printShaderInfoLog(vertexShader);
			checkAllGLErrors();
			std::terminate();
		}
	}
	checkAllGLErrors();
	return vertexShader;
}

GLuint compileFragmentShader(const std::string& shaderSource) noexcept
{
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* shaderSourcePtr = shaderSource.c_str();
	glShaderSource(fragmentShader, 1, &shaderSourcePtr, NULL);
	glCompileShader(fragmentShader);
	{
		int compileSuccess;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileSuccess);
		if (!compileSuccess) {
			printShaderInfoLog(fragmentShader);
			checkAllGLErrors();
			std::terminate();
		}
	}
	checkAllGLErrors();
	return fragmentShader;
}

void linkProgram(GLuint program) noexcept
{
	glLinkProgram(program);
	{
		GLint linkSuccess = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
		if (!linkSuccess) {
			printShaderInfoLog(program);
			checkAllGLErrors();
			std::terminate();
		}
	}
	checkAllGLErrors();
}

// Uniform setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void setUniform(int location, const mat4f& matrix) noexcept
{
	glUniformMatrix4fv(location, 1, false, matrix.glPtr());
}

void setUniform(GLuint shaderProgram, const string& name, const mat4f& matrix) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, matrix);
}

void setUniform(int location, const mat4f* matrixArray, size_t count) noexcept
{
	glUniformMatrix4fv(location, count, false, matrixArray[0].glPtr());
}

void setUniform(GLuint shaderProgram, const string& name, const mat4f* matrixArray, size_t count) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, matrixArray, count);
}


void setUniform(int location, const vec3f& vector) noexcept
{
	glUniform3fv(location, 1, vector.glPtr());
}

void setUniform(GLuint shaderProgram, const string& name, const vec3f& vector) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, vector);
}

void setUniform(int location, const vec3f* vectorArray, size_t count) noexcept
{
	glUniform3fv(location, count, vectorArray[0].glPtr());
}

void setUniform(GLuint shaderProgram, const string& name, const vec3f* vectorArray, size_t count) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, vectorArray, count);
}


void setUniform(int location, float f) noexcept
{
	glUniform1f(location, f);
}

void setUniform(GLuint shaderProgram, const string& name, float f) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, f);
}

void setUniform(int location, const float* floatArray, size_t count) noexcept
{
	glUniform1fv(location, count, floatArray);
}

void setUniform(GLuint shaderProgram, const string& name, const float* floatArray, size_t count) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, floatArray, count);
}


void setUniform(int location, int i) noexcept
{
	glUniform1i(location, i);
}

void setUniform(GLuint shaderProgram, const string& name, int i) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, i);
}

void setUniform(int location, const int* intArray, size_t count) noexcept
{
	glUniform1iv(location, count, intArray);
}

void setUniform(GLuint shaderProgram, const string& name, const int* intArray, size_t count) noexcept
{
	int loc = glGetUniformLocation(shaderProgram, name.c_str());
	setUniform(loc, intArray, count);
}

} // namespace gl

#include <sfz/MSVC12HackOFF.hpp>