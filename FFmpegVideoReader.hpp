#pragma once

#include <chrono>

#include "StreamVideoFrame.hpp"
#include "VideoReader.hpp"

/// An interface for a VideoReader class.
class FFmpegVideoReader final : public VideoReader {

public:

	/// Returns true if libav can open the video file at the provided path
	static bool canReadFile(const std::string& filename);

	/// Constructor
	/// \param filename Path of the video file to open
	FFmpegVideoReader(const std::string& filename);

	~FFmpegVideoReader();

	const std::shared_ptr<StreamVideoFrame>& getCurrentFrame() const override { return currentFrame; }

	const std::shared_ptr<StreamVideoFrame>& getNextFrame() override;

	double getFPS() const override { return fps; }

	int64_t getVideoLength() const override { return ctxt->streams[videoStream]->duration; }

	void seek(int64_t ts) override;

	int64_t clocksToTimestamp(clock_t c) const override;

	int64_t durationToTimestamp(const std::chrono::milliseconds& d) const override;

	clock_t timestampToClocks(int64_t ts) const override;

	std::chrono::milliseconds timestampToDuration(int64_t ts) const override;

	int64_t timestampToSeconds(int64_t ts) const override;

	// No copying
	FFmpegVideoReader(const FFmpegVideoReader&) = delete;
	FFmpegVideoReader& operator=(const FFmpegVideoReader&) = delete;

private:

	// FFmpeg structs and IDs

	AVFormatContext* ctxt;

	AVCodecContext* codecCtxt;

	SwsContext* swsCtxt;

	int videoStream;

	AVRational videoTimeBase;

	AVPacket currentPacket;

	/// The amount of the current packet that has already been processed
	int amountProcessed;

	/// A pointer to the current frame
	std::shared_ptr<StreamVideoFrame> currentFrame;

	/// Frame rate in frames per second (extracted from the video stream)
	double fps;

};
