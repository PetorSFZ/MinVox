#pragma once
#ifndef VOX_RENDERING_IN_GAME_PROFILER_HPP
#define VOX_RENDERING_IN_GAME_PROFILER_HPP

#include <chrono>
#include <cstddef> // size_t
#include <initializer_list>
#include <memory>
#include <string>

#include <sfz/MSVC12HackON.hpp>

namespace vox {

using std::unique_ptr;
using std::string;
using std::size_t;

// InGameProfiler
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

class InGameProfiler final {
public:

	// Constructors & destructors
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	InGameProfiler() = default; // Needed because of MSVC12
	InGameProfiler(const InGameProfiler&) = delete;
	InGameProfiler& operator= (const InGameProfiler&) = delete;

	InGameProfiler(InGameProfiler&& other) noexcept;
	InGameProfiler& operator= (InGameProfiler&& other) noexcept;
	InGameProfiler(std::initializer_list<string> list) noexcept;

	// Public methods
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	void startProfiling() noexcept;
	void endProfiling(size_t index) noexcept;

	// Getters
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

	inline size_t size() const noexcept { return mSize; }
	string& measurementName(size_t i) const noexcept;
	float latestMeasurement(size_t i) const noexcept;
	float averageMeasurement(size_t i) const noexcept;
	string& completeString(size_t i) const noexcept;

private:
	std::chrono::high_resolution_clock::time_point mStartTime, mEndTime;

	size_t mSize;
	unique_ptr<string[]> mMeasurementNames;
	unique_ptr<float[]> mLatestMeasurements;
	unique_ptr<size_t[]> mNumMeasurements;
	unique_ptr<float[]> mAverageMeasurements;
	unique_ptr<string[]> mCompleteStrings;
};

} // namespace vox

#include <sfz/MSVC12HackOFF.hpp>

#endif