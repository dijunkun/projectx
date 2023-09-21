#include "video_decoder_factory.h"

#include "ffmpeg/ffmpeg_video_decoder.h"
#include "log.h"
#include "nvcodec/nvidia_video_decoder.h"

VideoDecoderFactory::VideoDecoderFactory() {}

VideoDecoderFactory::~VideoDecoderFactory() {}

std::unique_ptr<VideoDecoder> VideoDecoderFactory::CreateVideoDecoder(
    bool hardware_acceleration) {
  if (hardware_acceleration) {
    if (CheckIsHardwareAccerlerationSupported()) {
      return std::make_unique<NvidiaVideoDecoder>(NvidiaVideoDecoder());
    } else {
      return nullptr;
    }
  } else {
    return std::make_unique<FfmpegVideoDecoder>(FfmpegVideoDecoder());
  }
}

bool VideoDecoderFactory::CheckIsHardwareAccerlerationSupported() {
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
}