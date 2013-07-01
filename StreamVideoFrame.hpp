#pragma once
#include "VideoFrame.hpp"

/// A video frame from a stream that contains additional information besides dimensions and pixel data.
class StreamVideoFrame : public VideoFrame {
public:
	/**
	 * \brief Initializes a video frame without zeroing it. It assumed that the video reader will fill it next.
	 * \param w Width of the frame
	 * \param h Height of the frame
	 * \param d Byte depth of each pixel
	 * \param presTS presentation timestamp
	 */
	StreamVideoFrame(size_t w, size_t h, size_t d, int64_t presTS)
		: VideoFrame(w, h, d, false), pts(presTS)
	{ }

	int64_t getPTS() const { return pts; }

private:
	int64_t pts; /// Presentation timestamp (when this frame should be shown)
};
