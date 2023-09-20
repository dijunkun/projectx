#include "video_encoder_factory.h"

#include "ffmpeg/ffmpeg_video_encoder.h"
#include "nvcodec/nvidia_video_encoder.h"

VideoEncoderFactory::VideoEncoderFactory() {}

VideoEncoderFactory::~VideoEncoderFactory() {}

VideoEncoder *VideoEncoderFactory::CreateVideoEncoder(
    bool hardware_acceleration) {
  if (hardware_acceleration) {
    return new NvidiaVideoEncoder();
  } else {
    return new FFmpegVideoEncoder();
  }
}
