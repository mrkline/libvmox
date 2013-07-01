#include "precomp.hpp" // Precompiled headers (all extrenal library headers)

#include "FFmpegVideoReader.hpp"
#include "MotionExtractor.hpp"
#include "VideoFrame.hpp"

int main()
{
	FFmpegVideoReader reader("myVideo.mp4");

	// Get the first frame so we know the video size
	auto frame = reader.getNextFrame(); // Returns a shared_ptr to a video frame

	// The motion extractor needs to know the frame size (to allocate internal buffers) and the frame rate.
	// Benchmarking is disabled here.
	MotionExtractor extractor(frame->getWidth(), frame->getHeight(), reader.getFPS(), false);

	// While the video isn't over, extract moving objects
	while (frame != nullptr) {

		// This mask is a 24-bit RGB image so that it can be used for both display and programmatic purposes.
		// The motion mask is contained in the blue channel - the red and green channels can be used as-desired
		// by the user.
		VideoFrame& motionMask = extractor.generateMotionMask(*frame);

		// Do something with the motion mask
	}

	return 0;
}
