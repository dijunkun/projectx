#include "video_decoder_factory.h"

#if __APPLE__
#include "ffmpeg/ffmpeg_video_decoder.h"
#include "openh264/openh264_decoder.h"
#else
#include "ffmpeg/ffmpeg_video_decoder.h"
#include "nvcodec/nvidia_video_decoder.h"
#include "openh264/openh264_decoder.h"
#endif

#include "log.h"

VideoDecoderFactory::VideoDecoderFactory() {}

VideoDecoderFactory::~VideoDecoderFactory() {}

std::unique_ptr<VideoDecoder> VideoDecoderFactory::CreateVideoDecoder(
    bool hardware_acceleration) {
#if __APPLE__
  return std::make_unique<OpenH264Decoder>(OpenH264Decoder());
  // return std::make_unique<FfmpegVideoDecoder>(FfmpegVideoDecoder());
#else
  if (hardware_acceleration) {
    if (CheckIsHardwareAccerlerationSupported()) {
      return std::make_unique<NvidiaVideoDecoder>(NvidiaVideoDecoder());
    } else {
      return nullptr;
    }
  } else {
    // return std::make_unique<FfmpegVideoDecoder>(FfmpegVideoDecoder());
    return std::make_unique<OpenH264Decoder>(OpenH264Decoder());
  }
#endif
}

bool VideoDecoderFactory::CheckIsHardwareAccerlerationSupported() {
#if __APPLE__
  return false;
#else
  CUresult cuResult;

  CUvideoctxlock cudaCtxLock;
  cuResult = cuvidCtxLockCreate(&cudaCtxLock, 0);
  if (cuResult != CUDA_SUCCESS) {
    LOG_WARN(
        "System not support hardware accelerated decode, use default software "
        "decoder");
    return false;
  }

  return true;
#endif
}