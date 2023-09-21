#include "video_encoder_factory.h"

#include "ffmpeg/ffmpeg_video_encoder.h"
#include "log.h"
#include "nvcodec/nvidia_video_encoder.h"

VideoEncoderFactory::VideoEncoderFactory() {}

VideoEncoderFactory::~VideoEncoderFactory() {}

std::unique_ptr<VideoEncoder> VideoEncoderFactory::CreateVideoEncoder(
    bool hardware_acceleration) {
  if (hardware_acceleration) {
    if (CheckIsHardwareAccerlerationSupported()) {
      return std::make_unique<NvidiaVideoEncoder>(NvidiaVideoEncoder());
    } else {
      return nullptr;
    }
  } else {
    return std::make_unique<FFmpegVideoEncoder>(FFmpegVideoEncoder());
  }
}

bool VideoEncoderFactory::CheckIsHardwareAccerlerationSupported() {
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
}