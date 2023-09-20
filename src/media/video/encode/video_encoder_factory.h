#ifndef _VIDEO_ENCODER_FACTORY_H_
#define _VIDEO_ENCODER_FACTORY_H_

#include "video_encoder.h"
class VideoEncoderFactory {
 public:
  VideoEncoderFactory();
  ~VideoEncoderFactory();

  static VideoEncoder *CreateVideoEncoder(bool hardware_acceleration);

 private:
  bool hardware_acceleration_ = false;
};

#endif