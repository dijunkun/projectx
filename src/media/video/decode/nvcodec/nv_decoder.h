#ifndef _NV_DECODER_H_
#define _NV_DECODER_H_

#include "NvDecoder.h"

class VideoDecoder {
 public:
  VideoDecoder();
  ~VideoDecoder();

  int Init();
  int Decode(const uint8_t* pData, int nSize);
  int GetFrame(uint8_t* yuv_data, uint32_t& width, uint32_t& height,
               uint32_t& size);

  NvDecoder* decoder = nullptr;
  bool get_first_keyframe_ = false;
  bool skip_frame_ = false;
  FILE* file_ = nullptr;
};

#endif