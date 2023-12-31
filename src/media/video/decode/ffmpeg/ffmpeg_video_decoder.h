#ifndef _FFMPEG_VIDEO_DECODER_H_
#define _FFMPEG_VIDEO_DECODER_H_

#ifdef _WIN32
extern "C" {
#include "libavcodec/avcodec.h"
};
#else
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
};
#endif
#endif

#include <functional>

#include "video_decoder.h"

class FfmpegVideoDecoder : public VideoDecoder {
 public:
  FfmpegVideoDecoder();
  virtual ~FfmpegVideoDecoder();

 public:
  int Init();
  int Decode(const uint8_t *data, int size,
             std::function<void(VideoFrame)> on_receive_decoded_frame);

 private:
  AVCodecID codec_id_;
  const AVCodec *codec_;
  AVCodecContext *codec_ctx_ = nullptr;
  AVPacket *packet_ = nullptr;
  AVFrame *frame_ = nullptr;
  AVFrame *frame_nv12_ = nullptr;
  struct SwsContext *img_convert_ctx = nullptr;

  VideoFrame *decoded_frame_ = nullptr;

  FILE *file_ = nullptr;
  bool first_ = false;
};

#endif