#pragma once

#include <memory>

#include "Exceptions.hpp"
#include "StreamVideoFrame.hpp"

/// An abstract VideoReader class.
class VideoReader {

public:

	// Initialize frame data to zero until it is actually set by a getNextFrame() call
	VideoReader() : frameWidth(0), frameHeight(0), frameDepth(0), aspectRatio(0.0f) { }

	/// Gets the current frame of video. This will initially return null until getNextFrame is called at least once.
	virtual const std::shared_ptr<StreamVideoFrame>& getCurrentFrame() const = 0;

	/// Gets the next frame in the video. Returns null when the video is over
	virtual const std::shared_ptr<StreamVideoFrame>& getNextFrame() = 0;

	/// Gets the frame rate of the video in frames per second
	virtual double getFPS() const = 0;

	/// Returns video length, in the same time step as frame timestamps.
	virtual int64_t getVideoLength() const = 0;

	/// Seeks to the given time stamp
	virtual void seek(int64_t ts) = 0;

	/// Converts clocks to the video's internal timestamp.
	/// \warning clock() may not update rapidly enough
	///          to track time between frames on some systems
	virtual int64_t clocksToTimestamp(clock_t c) const = 0;

	/// Converts a std::duration (in milliseconds) to the video's internal timestamp
	virtual int64_t durationToTimestamp(const std::chrono::milliseconds& d) const = 0;

	/// Converts the video's internal timestamp to clocks
	/// \warning clock() may not update rapidly enough
	///          to track time between frames on some systems
	virtual clock_t timestampToClocks(int64_t ts) const = 0;

	/// Converts the video's internal timestamp to a std::duration (in milliseconds)
	virtual std::chrono::milliseconds timestampToDuration(int64_t ts) const = 0;

	/// Converts a video's timestamp to seconds (rounded to nearest)
	virtual int64_t timestampToSeconds(int64_t ts) const = 0;

	/// Gets the width of the frames in the video stream.
	/// \warning getNextFrame must have been called at least once before calling this function.
	size_t getFrameWidth() const
	{
		if (frameWidth == 0) {
			throw Exceptions::InvalidOperationException(
				"getNextFrame() must be called before frame info can be retrieved.",
				__FUNCTION__);
		}
		return frameWidth;
	}

	/// Gets the height of the frames in the video stream.
	/// \warning getNextFrame must have been called at least once before calling this function.
	size_t getFrameHeight() const
	{
		if (frameHeight == 0) {
			throw Exceptions::InvalidOperationException(
				"getNextFrame() must be called before frame info can be retrieved.",
				__FUNCTION__);
		}
		return frameHeight;
	}

	/// Gets the bytes per pixel of the frames in the video stream.
	/// \warning getNextFrame must have been called at least once before calling this function.
	size_t getFrameDepth() const
	{
		if (frameDepth == 0) {
			throw Exceptions::InvalidOperationException(
				"getNextFrame() must be called before frame info can be retrieved.",
				__FUNCTION__);
		}
		return frameDepth;
	}

	/// Gets the aspect ratio of the frames in the video stream, taking both the frame dimensions
	/// and the pixel aspect ratio
	/// \warning getNextFrame must have been called at least once before calling this function.
	float getAspectRatio() const
	{
		if (aspectRatio == 0.0f) {
			throw Exceptions::InvalidOperationException(
				"getNextFrame() must be called before frame info can be retrieved.",
				__FUNCTION__);
		}
		return aspectRatio;
	}

protected:
	size_t frameWidth;
	size_t frameHeight;
	size_t frameDepth;
	float aspectRatio;
};
