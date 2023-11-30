/*
 * @Author: DI JUNKUN
 * @Date: 2023-11-24
 * Copyright (c) 2023 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _VIDEO_DECODER_H_
#define _VIDEO_DECODER_H_

#include <functional>

#include "video_frame.h"

class VideoDecoder {
 public:
  virtual int Init() = 0;
  virtual int Decode(
      const uint8_t *data, int size,
      std::function<void(VideoFrame)> on_receive_decoded_frame) = 0;
};

#endif