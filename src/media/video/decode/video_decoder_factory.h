#ifndef _VIDEO_DECODER_FACTORY_H_
#define _VIDEO_DECODER_FACTORY_H_

#include "video_decoder.h"
class VideoDecoderFactory {
 public:
  VideoDecoderFactory();
  ~VideoDecoderFactory();

  static VideoDecoder *CreateVideoDecoder(bool hardware_acceleration);

 private:
  bool hardware_acceleration_ = false;
};

#endif