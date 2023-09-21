#include "video_decoder_factory.h"

#include "ffmpeg/ffmpeg_video_decoder.h"
#include "nvcodec/nvidia_video_decoder.h"

VideoDecoderFactory::VideoDecoderFactory() {}

VideoDecoderFactory::~VideoDecoderFactory() {}

std::unique_ptr<VideoDecoder> VideoDecoderFactory::CreateVideoDecoder(
    bool hardware_acceleration) {
  if (hardware_acceleration) {
    return std::make_unique<NvidiaVideoDecoder>(NvidiaVideoDecoder());
  } else {
    return std::make_unique<FfmpegVideoDecoder>(FfmpegVideoDecoder());
  }
}