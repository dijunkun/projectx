#include "video_decoder_factory.h"

#include "ffmpeg/ffmpeg_video_decoder.h"
#include "nvcodec/nvidia_video_decoder.h"

VideoDecoderFactory::VideoDecoderFactory() {}

VideoDecoderFactory::~VideoDecoderFactory() {}

VideoDecoder *VideoDecoderFactory::CreateVideoDecoder(
    bool hardware_acceleration) {
  if (hardware_acceleration) {
    return new NvidiaVideoDecoder();
  } else {
    return new FfmpegVideoDecoder();
  }
}