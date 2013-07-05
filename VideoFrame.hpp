#pragma once

#include <cstring>

#include "Exceptions.hpp"

/// A frame of video with a width, height, depth, and data
class VideoFrame {
public:

	/**
	 * \brief Creates a frame from existing pixel data.
	 * \param pix The pixel data on which to base the frame
	 * \param w Width of the frame
	 * \param h Height of the frame
	 * \param d Byte depth of each pixel
	 * \param makeCopy true to make a copy of the data. If this is false, the frame is not responsible for managing
	 *                 the pixel memory
	 */
	VideoFrame(uint8_t* pix, size_t w, size_t h, size_t d, bool makeCopy)
		: pixels(pix),
		  width(w),
		  height(h),
		  depth(d),
		  totalSize(w * h * d),
		  ownsPixels(makeCopy)
	{
		if (makeCopy) {
			pixels = new uint8_t[width * height * depth];
			memcpy(pixels, pix, totalSize);
		}
	}

	/**
	 * \brief Creates a blank frame, memset with the given value
	 * \param w Width of the frame
	 * \param h Height of the frame
	 * \param d Byte depth of each pixel
	 * \param zero True to zero the frame, otherwise leave it uninitialized.
	 */
	VideoFrame(size_t w, size_t h, size_t d, bool zero = true)
		: pixels(nullptr),
		  width(w),
		  height(h),
		  depth(d),
		  totalSize(w * h * d),
		  ownsPixels(true)
	{
		pixels = new uint8_t[totalSize];
		if (zero)
			memset(pixels, 0, totalSize);
	}

	/// Constructs a video frame from another frame
	VideoFrame(const VideoFrame& other)
		: pixels(nullptr),
		  width(other.width),
		  height(other.height),
		  depth(other.depth),
		  totalSize(other.totalSize),
		  ownsPixels(true)
	{
		pixels = new uint8_t[totalSize];
		memcpy(pixels, other.pixels, totalSize);
	}

	virtual ~VideoFrame() { if(ownsPixels) delete[] pixels; }

	/// Memsets the frame to a given value (or a default of 0)
	void wipe(int memsetTo = 0)
	{
		memset(pixels, memsetTo, totalSize);
	}

	uint8_t* getPixels() { return pixels; }

	const uint8_t* getPixels() const { return pixels; }

	/// Gets a pixel at a given coordinate
	/// \warning Does not do bounds checking
	/// \returns The address of the first byte of the given pixel
	uint8_t* getPixel(size_t x, size_t y) { return &pixels[(y * width + x) * depth]; }

	/// Gets a pixel at a given coordinate
	/// \warning Does not do bounds checking
	/// \returns The address of the first byte of the given pixel
	const uint8_t* getPixel(size_t x, size_t y) const { return &pixels[(y * width + x) * depth]; }

	size_t getWidth() const { return width; };

	size_t getHeight() const { return height; };

	/// Returns the image's size, in bytes
	size_t getTotalSize() const {return totalSize; }

	size_t getBytesPerPixel() const { return depth; }

	VideoFrame& operator= (const VideoFrame& other)
	{
		if (width != other.width || height != other.height || depth != other.depth)
			throw Exceptions::InvalidOperationException("To copy from one frame to another,"
			                                            " frames must be the same dimensions.",
			                                            __FUNCTION__);

		memcpy(pixels, other.pixels, totalSize);
		return *this;
	}

private:

	uint8_t* pixels;
	size_t width;
	size_t height;
	size_t depth;
	size_t totalSize;
	bool ownsPixels;

};
