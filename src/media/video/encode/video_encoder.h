#ifndef _VIDEO_ENCODER_H_
#define _VIDEO_ENCODER_H_

#include <functional>

class VideoEncoder {
 public:
  virtual int Init() = 0;
  virtual int Encode(const uint8_t* pData, int nSize,
                     std::function<int(char* encoded_packets, size_t size)>
                         on_encoded_image) = 0;
  virtual int OnEncodedImage(char* encoded_packets, size_t size) = 0;
  virtual void ForceIdr() = 0;
};

#endif