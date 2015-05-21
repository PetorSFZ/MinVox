#include <rendering/InGameProfiler.hpp>

#include <sfz/Assert.hpp>

#include <sfz/MSVC12HackON.hpp>

namespace vox {

// InGameProfiler: Constructors & destructors
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

InGameProfiler::InGameProfiler(std::initializer_list<string> list) noexcept
:
	mSize{list.size()},
	mMeasurementNames{new string[mSize]},
	mLatestMeasurements{new float[mSize]},
	mNumMeasurements{new size_t[mSize]},
	mAverageMeasurements{new float[mSize]},
	mCompleteStrings{new string[mSize]}
{
	size_t i = 0;
	for (auto& str : list) {
		mMeasurementNames[i] = str;
		mLatestMeasurements[i] = 0.0f;
		mAverageMeasurements[i] = 0.0f;
		mNumMeasurements[i] = 0;
		++i;
	}
}

// InGameProfiler: Public methods
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

void InGameProfiler::startProfiling() noexcept
{
	mStartTime = std::chrono::high_resolution_clock::now();
}

void InGameProfiler::endProfiling(size_t index) noexcept
{
	sfz_assert_debug(index < mSize);
	mEndTime = std::chrono::high_resolution_clock::now();

	using FloatMilliSecondDuration = std::chrono::duration<float, std::milli>;
	float t = std::chrono::duration_cast<FloatMilliSecondDuration>(mEndTime - mStartTime).count();

	mLatestMeasurements[index] = t;
	mNumMeasurements[index] += 1;
	mAverageMeasurements[index] = (mAverageMeasurements[index]*(mNumMeasurements[index]-1) + t)
	                            / (float)mNumMeasurements[index];

	mCompleteStrings[index].clear();
	mCompleteStrings[index] += mMeasurementNames[index];
	mCompleteStrings[index] += ": ";
	mCompleteStrings[index] += std::to_string(mLatestMeasurements[index]);
	mCompleteStrings[index] += "ms, avg: ";
	mCompleteStrings[index] += std::to_string(mAverageMeasurements[index]);
	mCompleteStrings[index] += "ms";
}

// InGameProfiler: Getters
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

string& InGameProfiler::measurementName(size_t i) const noexcept
{
	sfz_assert_debug(i < mSize);
	return mMeasurementNames[i];
}

float InGameProfiler::latestMeasurement(size_t i) const noexcept
{
	sfz_assert_debug(i < mSize);
	return mLatestMeasurements[i];
}

float InGameProfiler::averageMeasurement(size_t i) const noexcept
{
	sfz_assert_debug(i < mSize);
	return mAverageMeasurements[i];
}

string& InGameProfiler::completeString(size_t i) const noexcept
{
	sfz_assert_debug(i < mSize);
	return mCompleteStrings[i];
}

}

#include <sfz/MSVC12HackOFF.hpp>