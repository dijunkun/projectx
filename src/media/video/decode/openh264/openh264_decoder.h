/*
 * @Author: DI JUNKUN
 * @Date: 2023-11-03
 * Copyright (c) 2023 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _OPENH264_DECODER_H_
#define _OPENH264_DECODER_H_

#include <wels/codec_api.h>
#include <wels/codec_app_def.h>
#include <wels/codec_def.h>
#include <wels/codec_ver.h>

#include <functional>

#include "video_decoder.h"

class OpenH264Decoder : public VideoDecoder {
 public:
  OpenH264Decoder();
  virtual ~OpenH264Decoder();

 public:
  int Init();
  int Decode(const uint8_t* data, int size,
             std::function<void(VideoFrame)> on_receive_decoded_frame);

 private:
  ISVCDecoder* openh264_decoder_ = nullptr;
  bool get_first_keyframe_ = false;
  bool skip_frame_ = false;
  FILE* nv12_stream_ = nullptr;
  FILE* h264_stream_ = nullptr;
  uint8_t* decoded_frame_ = nullptr;
  int decoded_frame_size_ = 0;
  uint8_t* nv12_frame_ = nullptr;
  unsigned char* pData[3] = {};
  int frame_width_ = 1280;
  int frame_height_ = 720;

  uint8_t* pData_tmp = nullptr;
  uint8_t* pData_tmp_2 = nullptr;
};

#endif