#include "video_encoder_factory.h"

#if __APPLE__
#include "ffmpeg/ffmpeg_video_encoder.h"
#else
#include "ffmpeg/ffmpeg_video_encoder.h"
#include "nvcodec/nvidia_video_encoder.h"
#include "openh264/openh264_encoder.h"
#endif

#include "log.h"

VideoEncoderFactory::VideoEncoderFactory() {}

VideoEncoderFactory::~VideoEncoderFactory() {}

std::unique_ptr<VideoEncoder> VideoEncoderFactory::CreateVideoEncoder(
    bool hardware_acceleration) {
#if __APPLE__
  return std::make_unique<FFmpegVideoEncoder>(FFmpegVideoEncoder());
#else
  if (hardware_acceleration) {
    if (CheckIsHardwareAccerlerationSupported()) {
      return std::make_unique<NvidiaVideoEncoder>(NvidiaVideoEncoder());
    } else {
      return nullptr;
    }
  } else {
    // return std::make_unique<FFmpegVideoEncoder>(FFmpegVideoEncoder());
    return std::make_unique<OpenH264Encoder>(OpenH264Encoder());
  }
#endif
}

bool VideoEncoderFactory::CheckIsHardwareAccerlerationSupported() {
#if __APPLE__
  return false;
#else
  CUresult cuResult;
  NV_ENCODE_API_FUNCTION_LIST functionList = {NV_ENCODE_API_FUNCTION_LIST_VER};

  cuResult = cuInit(0);
  if (cuResult != CUDA_SUCCESS) {
    LOG_WARN(
        "System not support hardware accelerated encode, use default software "
        "encoder");
    return false;
  }

  NVENCSTATUS nvEncStatus = NvEncodeAPICreateInstance(&functionList);
  if (nvEncStatus != NV_ENC_SUCCESS) {
    LOG_WARN(
        "System not support hardware accelerated encode, use default software "
        "encoder");
    return false;
  }

  return true;
#endif
}