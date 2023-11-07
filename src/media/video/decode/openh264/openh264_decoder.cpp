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

#define SAVE_NV12_STREAM 0
#define SAVE_H264_STREAM 0

static const int YUV420P_BUFFER_SIZE = 1280 * 720 * 3 / 2;

void CopyYUVWithStride(uint8_t *srcY, uint8_t *srcU, uint8_t *srcV, int width,
                       int height, int strideY, int strideU, int strideV,
                       uint8_t *dst) {
  int actualWidth = width;
  int actualHeight = height;

  int actualStrideY = actualWidth;
  int actualStrideU = actualWidth / 2;
  int actualStrideV = actualWidth / 2;

  for (int row = 0; row < actualHeight; row++) {
    memcpy(dst, srcY, actualStrideY);
    srcY += strideY;
    dst += actualStrideY;
  }

  for (int row = 0; row < actualHeight / 2; row++) {
    memcpy(dst, srcU, actualStrideU);
    srcU += strideU;
    dst += actualStrideU;
  }

  for (int row = 0; row < actualHeight / 2; row++) {
    memcpy(dst, srcV, actualStrideV);
    srcV += strideV;
    dst += actualStrideV;
  }
}

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
  if (openh264_decoder_) {
    openh264_decoder_->Uninitialize();
    WelsDestroyDecoder(openh264_decoder_);
  }

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

  if (pData_tmp) {
    delete pData_tmp;
    pData_tmp = nullptr;
  }

  if (SAVE_H264_STREAM && h264_stream_) {
    fflush(h264_stream_);
    h264_stream_ = nullptr;
  }

  if (SAVE_NV12_STREAM && nv12_stream_) {
    fflush(nv12_stream_);
    nv12_stream_ = nullptr;
  }
}

int OpenH264Decoder::Init() {
  if (SAVE_NV12_STREAM) {
    nv12_stream_ = fopen("nv12_receive_.yuv", "w+b");
    if (!nv12_stream_) {
      LOG_WARN("Fail to open nv12_receive_.yuv");
    }
  }

  if (SAVE_NV12_STREAM) {
    h264_stream_ = fopen("h264_receive.h264", "w+b");
    if (!h264_stream_) {
      LOG_WARN("Fail to open h264_receive.h264");
    }
  }

  frame_width_ = 1280;
  frame_height_ = 720;

  decoded_frame_size_ = YUV420P_BUFFER_SIZE;
  decoded_frame_ = new uint8_t[YUV420P_BUFFER_SIZE];
  nv12_frame_ = new uint8_t[YUV420P_BUFFER_SIZE];
  // pData[0] = new uint8_t[1280 * 720];
  // pData[1] = new uint8_t[1280 * 720 / 4];
  // pData[2] = new uint8_t[1280 * 720 / 4];

  pData_tmp = new uint8_t[frame_width_ * frame_height_ * 3 / 2];
  // *pData = pData_tmp;
  // *(pData + 1) = pData_tmp + frame_width_ * frame_height_;
  // *(pData + 2) = pData_tmp + (frame_width_ * frame_height_ * 5) / 4;

  if (WelsCreateDecoder(&openh264_decoder_) != 0) {
    LOG_ERROR("Failed to create OpenH264 decoder");
    return -1;
  }

  SDecodingParam sDecParam;

  memset(&sDecParam, 0, sizeof(SDecodingParam));
  sDecParam.uiTargetDqLayer = UCHAR_MAX;
  sDecParam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
  sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

  int32_t iRet = openh264_decoder_->Initialize(&sDecParam);

  int trace_level = WELS_LOG_WARNING;
  openh264_decoder_->SetOption(DECODER_OPTION_TRACE_LEVEL, &trace_level);

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

  if (SAVE_H264_STREAM) {
    fwrite((unsigned char *)data, 1, size, h264_stream_);
  }

  SBufferInfo sDstBufInfo = {0};
  memset(&sDstBufInfo, 0, sizeof(SBufferInfo));

  unsigned char *dst[3];

  int iRet = openh264_decoder_->DecodeFrame2(data, size, dst, &sDstBufInfo);
  // int iRet =
  //     openh264_decoder_->DecodeFrameNoDelay(data, size, dst, &sDstBufInfo);

  if (iRet != 0) {
    return -1;
  }

  // int num_of_buffer = 0;
  // iRet = openh264_decoder_->GetOption(
  //     DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER, &num_of_buffer);

  // LOG_ERROR("Number of buffer {} {}", num_of_buffer, iRet);

  // iRet = openh264_decoder_->FlushFrame(dst, &sDstBufInfo);
  // if (iRet != 0) {
  //   LOG_ERROR("FlushFrame state: {}", iRet);
  //   return -1;
  // }

  if (1) {
    if (on_receive_decoded_frame) {
      CopyYUVWithStride(
          dst[0], dst[1], dst[2], sDstBufInfo.UsrData.sSystemBuffer.iWidth,
          sDstBufInfo.UsrData.sSystemBuffer.iHeight,
          sDstBufInfo.UsrData.sSystemBuffer.iStride[0],
          sDstBufInfo.UsrData.sSystemBuffer.iStride[1],
          sDstBufInfo.UsrData.sSystemBuffer.iStride[1], decoded_frame_);

      if (SAVE_NV12_STREAM) {
        fwrite((unsigned char *)decoded_frame_, 1,
               frame_width_ * frame_height_ * 3 / 2, nv12_stream_);
      }
      YUV420ToNV12PFFmpeg(decoded_frame_, frame_width_, frame_height_,
                          nv12_frame_);

      VideoFrame decoded_frame(nv12_frame_,
                               frame_width_ * frame_height_ * 3 / 2,
                               frame_width_, frame_height_);

      on_receive_decoded_frame(decoded_frame);
      if (SAVE_NV12_STREAM) {
        fwrite((unsigned char *)decoded_frame.Buffer(), 1, decoded_frame.Size(),
               nv12_stream_);
      }
    }
  }

  return 0;
}