#include "video_encoder_factory.h"

#include "ffmpeg/ffmpeg_video_encoder.h"
#include "nvcodec/nvidia_video_encoder.h"

VideoEncoderFactory::VideoEncoderFactory() {}

VideoEncoderFactory::~VideoEncoderFactory() {}

std::unique_ptr<VideoEncoder> VideoEncoderFactory::CreateVideoEncoder(
    bool hardware_acceleration) {
  if (hardware_acceleration) {
    return std::make_unique<NvidiaVideoEncoder>(NvidiaVideoEncoder());
  } else {
    return std::make_unique<FFmpegVideoEncoder>(FFmpegVideoEncoder());
  }
}
