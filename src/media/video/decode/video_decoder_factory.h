#ifndef _VIDEO_DECODER_FACTORY_H_
#define _VIDEO_DECODER_FACTORY_H_

#include <memory>

#include "video_decoder.h"
class VideoDecoderFactory {
 public:
  VideoDecoderFactory();
  ~VideoDecoderFactory();

  static std::unique_ptr<VideoDecoder> CreateVideoDecoder(
      bool hardware_acceleration);

  static bool CheckIsHardwareAccerlerationSupported();
};

#endif