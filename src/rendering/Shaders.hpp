#pragma once
#ifndef VOX_RENDERING_SHADERS_HPP
#define VOX_RENDERING_SHADERS_HPP

#include "sfz/GL.hpp"

#include <sfz/MSVC12HackON.hpp>

namespace vox {

/**
 * Shaders are compiled and returned by the functions in this class. The motivation for this is
 * that we currently don't have a good way of storing the shader source outside a c++ source file
 * without causing pain when the shaders are changed. This way we at least only need to recompile
 * one object file and not the whole Main.cpp when shaders are changed.
 */

GLuint compileStandardShaderProgram() noexcept;

GLuint compileShadowMapShaderProgram() noexcept;

GLuint compilePostProcessShaderProgram() noexcept;

GLuint compileGBufferGenShaderProgram() noexcept;

GLuint compileLightingShaderProgram() noexcept;

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>
#endif