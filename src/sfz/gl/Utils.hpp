#pragma once
#ifndef SFZ_GL_UTILS_HPP
#define SFZ_GL_UTILS_HPP

#include <sfz/Math.hpp>
#include "sfz/GL.hpp"
#include <iostream>
#include <string>
#include <exception> // std::terminate()

namespace gl {

/**
 * @brief Checks the latest error returned by glGetError().
 * @return whether an error was found or not
 */
bool checkGLError();

/**
 * @brief Checks all errors returned by glGetError() until GL_NO_ERROR is returned.
 * @return whether any errors where found or not
 */
bool checkAllGLErrors();

/**
 * @brief Prints the error log returned by glShaderInfoLog().
 */
void printShaderInfoLog(GLuint program);

/**
 * @brief Creates and compiles a vertex shader with the specified source.
 * Terminates program if shader compilation failed.
 * @param shaderSource the source code of the vertex shader
 * @return the newly created and compiled vertex shader object
 */
GLuint compileVertexShader(const std::string& shaderSource);

/**
 * @brief Creates and compiles a fragment shader with the specified source.
 * Terminates program if shader compilation failed.
 * @param shaderSource the source code of the fragment shader
 * @return the newly created and compiled fragment shader object
 */
GLuint compileFragmentShader(const std::string& shaderSource);

/**
 * @brief Links the specified program.
 * Terminates program if linking failed.
 * @param program the program
 */
void linkProgram(GLuint program);

// Uniform setters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void setUniform(int location, const sfz::mat4f& matrix);
void setUniform(GLuint shaderProgram, const std::string& name, const sfz::mat4f& matrix);

void setUniform(int location, const sfz::vec3f& vector);
void setUniform(GLuint shaderProgram, const std::string& name, const sfz::vec3f& vector);

void setUniform(int location, int i);
void setUniform(GLuint shaderProgram, const std::string& name, int i);

} // namespace gl

#endif
