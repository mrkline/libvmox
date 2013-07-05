#include "precomp.hpp"
#include "FFmpegVideoReader.hpp"

#include "Exceptions.hpp"

using namespace std;

namespace {

// True if libavformat hasn't been initialized yet
bool needsInit = true;

inline void init()
{
	av_register_all();
	needsInit = false;
}

} // end anonymous namespace

bool FFmpegVideoReader::canReadFile(const string& filename)
{
	// If needed, do global initialization
	if (needsInit)
		init();

	AVFormatContext* ctxt = nullptr;
	const int ret = avformat_open_input(&ctxt, filename.c_str(), nullptr, nullptr);
	if (ret == 0)
		avformat_close_input(&ctxt);
	return ret == 0;

	/// \todo Add more? Just construct a reader?
}

FFmpegVideoReader::FFmpegVideoReader(const string& filename)
	: ctxt(nullptr),
	  codecCtxt(nullptr),
	  swsCtxt(nullptr),
	  videoStream(-1),
	  videoTimeBase(),
	  currentPacket(),
	  amountProcessed(0),
	  currentFrame(),
	  fps(-1)
{
	// If needed, do global initialization
	if (needsInit)
		init();

	// Code is hobbled together from various online tutorials and ffmpeg docs.

	// Open the file
	if (avformat_open_input(&ctxt, filename.c_str(), nullptr, nullptr) != 0)
		throw Exceptions::IOException("Cannot open video file", __FUNCTION__);

	// See if we have any streams (we should)
	if (avformat_find_stream_info(ctxt, nullptr) < 0) {
		avformat_close_input(&ctxt);
		throw Exceptions::IOException("Could not find stream info for video file", __FUNCTION__);
	}

	// Find the first video stream in the file and get a pointer to its codec context
	videoStream = -1;
	for (unsigned int i = 0; i < ctxt->nb_streams; ++i) {
		if (ctxt->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}
	if (videoStream == -1) {
		avformat_close_input(&ctxt);
		throw Exceptions::IOException("No video stream could be found in the file.", __FUNCTION__);
	}

	codecCtxt = ctxt->streams[videoStream]->codec;

	// Find the decoder for the video stream
	AVCodec* codec = avcodec_find_decoder(codecCtxt->codec_id);
	if (codec == nullptr) {
		avformat_close_input(&ctxt);
		throw Exceptions::IOException("No decoder could be found for the video stream.", __FUNCTION__);
	}

	// Inform the codec that we can handle truncated bitstreams
	if (codec->capabilities & CODEC_CAP_TRUNCATED)
		codecCtxt->flags |= CODEC_FLAG_TRUNCATED;

	if (avcodec_open2(codecCtxt, codec, 0) < 0) {
		avformat_close_input(&ctxt);
		throw Exceptions::IOException("The codec for the video stream could not be opened.", __FUNCTION__);
	}

	// Get the frame rate and time base
	AVRational& rate = ctxt->streams[videoStream]->r_frame_rate;
	fps = av_q2d(rate);
	videoTimeBase = ctxt->streams[videoStream]->time_base;

	// Zero out the current packet
	memset(&currentPacket, 0, sizeof(currentPacket));
}

FFmpegVideoReader::~FFmpegVideoReader()
{
	sws_freeContext(swsCtxt);
	avformat_close_input(&ctxt);
	av_free_packet(&currentPacket);
}

const shared_ptr<StreamVideoFrame>& FFmpegVideoReader::getNextFrame()
{
	std::shared_ptr<AVFrame> frame(avcodec_alloc_frame(), &av_free);

	// Read packets until we can form a frame from it
	int frameAvailable = 0;
	do {
		// Read a new packet if we've read everything in the current one
		if (amountProcessed >= currentPacket.size) {
			do {
				amountProcessed = 0;
				// Free the last packet
				av_free_packet(&currentPacket);
				if (av_read_frame(ctxt, &currentPacket) < 0) {
					// EOF
					currentFrame = nullptr;
					return currentFrame;
				}
			} while (currentPacket.stream_index != videoStream);
		}

		// Decode the part of the packet that hasn't been decoded yet
		AVPacket framePacket;
		framePacket = currentPacket;
		framePacket.data = currentPacket.data + amountProcessed;
		framePacket.size = currentPacket.size - amountProcessed;

		const int decode_ret = avcodec_decode_video2(codecCtxt, frame.get(), &frameAvailable, &framePacket);
		if (decode_ret < 0)
			throw Exceptions::IOException("Could not decode frame", __FUNCTION__);

		// Track how much of the packet we've decoded
		amountProcessed += decode_ret;
	} while(!frameAvailable);

	// Convert and return frame
	/// \todo Why do we have to flip red and blue? The answer probably has to do with endianness
	if (swsCtxt == nullptr) {
		swsCtxt = sws_getContext(frame->width, frame->height, (PixelFormat)frame->format,
		                         frame->width, frame->height, PIX_FMT_BGR24, SWS_FAST_BILINEAR,
		                         nullptr, nullptr, nullptr);
		if (swsCtxt == nullptr)
			throw Exceptions::IOException("Error while calling sws_getContext", __FUNCTION__);
	}

	AVPicture pic;
	avpicture_alloc(&pic, PIX_FMT_RGB24, frame->width, frame->height);
	sws_scale(swsCtxt, frame->data, frame->linesize, 0, frame->height, pic.data, pic.linesize);

	// Set the VideoReader frame info
	frameWidth = frame->width;
	frameHeight = frame->height;
	frameDepth = 3;
	AVRational ar;
	ar.num = frameWidth;
	ar.den = frameHeight;
	const AVRational& sar = codecCtxt->sample_aspect_ratio;
	if (sar.num != 0 && sar.den != 0) // might not have to check den
		ar = av_mul_q(ar, sar);
	aspectRatio = (float)av_q2d(ar);

	// \todo Should we try to pick between pts and pkt_pts here? Using pkt_pts for now
	currentFrame = make_shared<StreamVideoFrame>(frame->width, frame->height, 3, frame->pkt_pts);

	// If the stride is equal to the frame width * 3, we can just copy the whole thing.
	// Otherwise we need to copy row by row
	const int rowLen = frame->width * 3;
	if (pic.linesize[0] == rowLen) {
		memcpy(currentFrame->getPixels(), pic.data[0], rowLen * frame->height);
	}
	else {
		unsigned char* src = pic.data[0];
		unsigned char* dest = currentFrame->getPixels();
		for (int h = 0; h < frame->height; ++h, dest += rowLen, src += pic.linesize[0])
			memcpy(dest, src, rowLen);
	}

	avpicture_free(&pic);

	return currentFrame;
}

void FFmpegVideoReader::seek(int64_t ts)
{
	if (av_seek_frame(ctxt, videoStream, ts, 0) < 0)
		throw Exceptions::IOException("Could not seek to the requested time stamp", __FUNCTION__);
	avcodec_flush_buffers(codecCtxt);
}

int64_t FFmpegVideoReader::clocksToTimestamp(clock_t c) const
{
	static AVRational clockBase = {1, CLOCKS_PER_SEC};
	return av_rescale_q(c, clockBase, videoTimeBase);

}

int64_t FFmpegVideoReader::durationToTimestamp(const std::chrono::milliseconds& d) const
{
	static AVRational msBase = {1, 1000};
	return av_rescale_q(d.count(), msBase, videoTimeBase);
}

clock_t FFmpegVideoReader::timestampToClocks(int64_t ts) const
{
	static AVRational clockBase = {1, CLOCKS_PER_SEC};
	return (clock_t)av_rescale_q(ts, videoTimeBase, clockBase);
}

std::chrono::milliseconds FFmpegVideoReader::timestampToDuration(int64_t ts) const
{
	static AVRational msBase = {1, 1000};
	return std::chrono::milliseconds(av_rescale_q(ts, videoTimeBase, msBase));
}

int64_t FFmpegVideoReader::timestampToSeconds(int64_t ts) const
{
	return av_rescale(ts, videoTimeBase.num, videoTimeBase.den);
}
