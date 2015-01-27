#pragma once
#ifndef VOX_IO_IO_UTILS_HPP
#define VOX_IO_IO_UTILS_HPP

#include <string>
#include <cstdio>
#include <exception>

#include "sfz/SDL.hpp"

namespace vox {

const std::string& basePath();
const std::string& assetsPath();
bool exists(const std::string& path);
bool createDirectory(const std::string& path);

} // namespace vox

#endif