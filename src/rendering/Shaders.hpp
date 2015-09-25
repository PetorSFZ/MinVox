#pragma once
#ifndef VOX_RENDERING_SHADERS_HPP
#define VOX_RENDERING_SHADERS_HPP

#include <sfz/gl/Program.hpp>

namespace vox {

using gl::Program;

/**
 * Shaders are compiled and returned by the functions in this file. The motivation for this is
 * that we currently don't have a good way of storing the shader source outside a c++ source file
 * without causing pain when the shaders are changed. This way we at least only need to recompile
 * one object file.
 */

Program compileShadowMapShaderProgram() noexcept;

Program compileGBufferGenShaderProgram() noexcept;

Program compileDirectionalLightingStencilShaderProgram() noexcept;

Program compileDirectionalLightingShaderProgram() noexcept;

Program compileGlobalLightingShaderProgram() noexcept;

Program compileOutputSelectShaderProgram() noexcept;

} // namespace vox


#endif