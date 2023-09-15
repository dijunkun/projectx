#include "nv_decoder.h"

#include "log.h"

#define SAVE_ENCODER_STREAM 0

VideoDecoder::VideoDecoder() {
  if (SAVE_ENCODER_STREAM) {
    file_ = fopen("decode_stream.h264", "w+b");
    if (!file_) {
      LOG_WARN("Fail to open stream.h264");
    }
  }
}
VideoDecoder::~VideoDecoder() {
  if (SAVE_ENCODER_STREAM && file_) {
    fflush(file_);
    fclose(file_);
    file_ = nullptr;
  }
}

int VideoDecoder::Init() {
  ck(cuInit(0));
  int nGpu = 0;
  int iGpu = 0;

  ck(cuDeviceGetCount(&nGpu));
  if (nGpu < 1) {
    return -1;
  }

  CUdevice cuDevice;
  cuDeviceGet(&cuDevice, iGpu);

  CUcontext cuContext = NULL;
  cuCtxCreate(&cuContext, 0, cuDevice);
  if (!cuContext) {
    return -1;
  }

  decoder = new NvDecoder(cuContext, false, cudaVideoCodec_H264, true);
  return 0;
}

int VideoDecoder::Decode(
    const uint8_t *data, int size,
    std::function<void(VideoFrame)> on_receive_decoded_frame) {
  if (!decoder) {
    return -1;
  }

  if ((*(data + 4) & 0x1f) == 0x07) {
    // LOG_WARN("Receive key frame");
  }

  if (SAVE_ENCODER_STREAM) {
    fwrite((unsigned char *)data, 1, size, file_);
  }

  int num_frame_returned = decoder->Decode(data, size);

  for (size_t i = 0; i < num_frame_returned; ++i) {
    cudaVideoSurfaceFormat format = decoder->GetOutputFormat();
    if (format == cudaVideoSurfaceFormat_NV12) {
      uint8_t *data = nullptr;
      data = decoder->GetFrame();
      if (data) {
        if (on_receive_decoded_frame) {
          VideoFrame decoded_frame(
              data, decoder->GetWidth() * decoder->GetHeight() * 3 / 2,
              decoder->GetWidth(), decoder->GetHeight());
          on_receive_decoded_frame(decoded_frame);
        }
      }
    }
  }

  return -1;
}