#include "openh264_decoder.h"

#include <cstring>

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};
#ifdef __cplusplus
};
#endif

#define SAVE_DECODER_STREAM 0
static const int YUV420P_BUFFER_SIZE = 1280 * 720 * 3 / 2;

int YUV420ToNV12PFFmpeg(unsigned char *src_buffer, int width, int height,
                        unsigned char *dst_buffer) {
  AVFrame *Input_pFrame = av_frame_alloc();
  AVFrame *Output_pFrame = av_frame_alloc();
  struct SwsContext *img_convert_ctx = sws_getContext(
      width, height, AV_PIX_FMT_YUV420P, 1280, 720, AV_PIX_FMT_NV12,
      SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

  av_image_fill_arrays(Input_pFrame->data, Input_pFrame->linesize, src_buffer,
                       AV_PIX_FMT_YUV420P, width, height, 1);
  av_image_fill_arrays(Output_pFrame->data, Output_pFrame->linesize, dst_buffer,
                       AV_PIX_FMT_NV12, 1280, 720, 1);

  sws_scale(img_convert_ctx, (uint8_t const **)Input_pFrame->data,
            Input_pFrame->linesize, 0, height, Output_pFrame->data,
            Output_pFrame->linesize);

  if (Input_pFrame) av_free(Input_pFrame);
  if (Output_pFrame) av_free(Output_pFrame);
  if (img_convert_ctx) sws_freeContext(img_convert_ctx);

  return 0;
}

OpenH264Decoder::OpenH264Decoder() {}
OpenH264Decoder::~OpenH264Decoder() {
  if (nv12_frame_) {
    delete nv12_frame_;
  }

  if (pData[0]) {
    delete pData[0];
  }

  if (pData[1]) {
    delete pData[1];
  }

  if (pData[2]) {
    delete pData[2];
  }
}

int OpenH264Decoder::Init() {
  SEncParamExt sParam;
  sParam.iPicWidth = 1280;
  sParam.iPicHeight = 720;
  sParam.iTargetBitrate = 1000;
  sParam.iTemporalLayerNum = 1;
  sParam.fMaxFrameRate = 30;
  sParam.iSpatialLayerNum = 1;

  decoded_frame_size_ = YUV420P_BUFFER_SIZE;
  decoded_frame_ = new uint8_t[YUV420P_BUFFER_SIZE];
  nv12_frame_ = new uint8_t[YUV420P_BUFFER_SIZE];
  pData[0] = new uint8_t[1280 * 720];
  pData[1] = new uint8_t[1280 * 720];
  pData[2] = new uint8_t[1280 * 720];

  if (WelsCreateDecoder(&openh264_decoder_) != 0) {
    LOG_ERROR("Failed to create OpenH264 decoder");
    return -1;
  }

  SDecodingParam sDecParam;

  memset(&sDecParam, 0, sizeof(SDecodingParam));
  sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
  sDecParam.bParseOnly = false;

  int32_t iRet = openh264_decoder_->Initialize(&sDecParam);

  LOG_ERROR("inited decoded_frame_size_ {}", decoded_frame_size_);
  LOG_ERROR("inited");
  printf("1 this is %p\n", this);
  return 0;
}

int OpenH264Decoder::Decode(
    const uint8_t *data, int size,
    std::function<void(VideoFrame)> on_receive_decoded_frame) {
  if (!openh264_decoder_) {
    return -1;
  }

  SBufferInfo sDstBufInfo;
  memset(&sDstBufInfo, 0, sizeof(SBufferInfo));

  int32_t iRet =
      openh264_decoder_->DecodeFrameNoDelay(data, size, pData, &sDstBufInfo);

  if (iRet != 0) {
    return -1;
  }

  if (sDstBufInfo.iBufferStatus == 1) {
    if (on_receive_decoded_frame) {
      memcpy(decoded_frame_, pData[0], frame_width_ * frame_height_);
      memcpy(decoded_frame_ + frame_width_ * frame_height_, pData[1],
             frame_width_ * frame_height_ / 2);
      memcpy(decoded_frame_ + frame_width_ * frame_height_ * 3 / 2, pData[2],
             frame_width_ * frame_height_ / 2);
      YUV420ToNV12PFFmpeg(decoded_frame_, frame_width_, frame_height_,
                          nv12_frame_);

      VideoFrame decoded_frame(nv12_frame_,
                               frame_width_ * frame_height_ * 3 / 2,
                               frame_width_, frame_height_);
      on_receive_decoded_frame(decoded_frame);
      if (SAVE_DECODER_STREAM) {
        fwrite((unsigned char *)decoded_frame.Buffer(), 1, decoded_frame.Size(),
               file_);
      }
    }
  }

  return 0;
}