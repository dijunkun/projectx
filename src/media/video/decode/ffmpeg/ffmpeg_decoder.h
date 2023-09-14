#ifndef _FFMPEG_DECODER_H_
#define _FFMPEG_DECODER_H_

extern "C" {
// 编解码
#include "libavcodec/avcodec.h"
}

#include "PacketQueue.h"
#include "cb/FrameDataCallback.h"

class VideoDecoder {
 public:
  VideoDecoder(PacketQueue *packetQueue);
  ~VideoDecoder();

 public:
  int Init();
  int Decode(const uint8_t *pData, int nSize);
  int GetFrame(uint8_t *yuv_data, uint32_t &width, uint32_t &height,
               uint32_t &size);

  bool open(unsigned int frameRate, unsigned int profile, unsigned int level,
            char *sps, unsigned int spsLen, char *pps, unsigned int ppsLen);

  void close();

  void decode();

  static void *_decode(void *self) {
    static_cast<VideoDecoder *>(self)->decode();
    return nullptr;
  }

  void setFrameDataCallback(FrameDataCallback *frameDataCallback);

 private:
  PacketQueue *pPacketQueue;
  AVCodecContext *pVideoAVCodecCtx;
  AVFrame *pFrame;

  bool volatile isDecoding;
  pthread_t decodeThread;
  pthread_mutex_t *pFrameDataCallbackMutex;
  FrameDataCallback *pFrameDataCallback;

  char *pSPS;
  unsigned int volatile gSPSLen;
  char *pPPS;
  unsigned int volatile gPPSLen;

  bool volatile isFirstIDR;

  unsigned int gFrameRate;
};

#endif