#ifndef _VIDEO_ENCODER_FACTORY_H_
#define _VIDEO_ENCODER_FACTORY_H_

#include "video_encoder.h"
class VideoEncoderFactory {
 public:
  VideoEncoderFactory();
  ~VideoEncoderFactory();

  static std::unique_ptr<VideoEncoder> CreateVideoEncoder(
      bool hardware_acceleration);

  static bool CheckIsHardwareAccerlerationSupported();
};

#endif