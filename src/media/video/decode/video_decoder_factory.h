#ifndef _VIDEO_DECODER_FACTORY_H_
#define _VIDEO_DECODER_FACTORY_H_

#include "video_decoder.h"
class VideoDecoderFactory {
 public:
  VideoDecoderFactory();
  ~VideoDecoderFactory();

  static std::unique_ptr<VideoDecoder> CreateVideoDecoder(
      bool hardware_acceleration);
};

#endif