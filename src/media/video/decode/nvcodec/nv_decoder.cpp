#include "nv_decoder.h"

#include "log.h"

VideoDecoder::VideoDecoder() {}
VideoDecoder::~VideoDecoder() {}

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

int VideoDecoder::Decode(const uint8_t *pData, int nSize) {
  if (!decoder) {
    return -1;
  }

  if ((*(pData + 4) & 0x1f) == 0x07) {
    // LOG_WARN("Receive key frame");
  }

  int ret = decoder->Decode(pData, nSize);
  return ret;
}

int VideoDecoder::GetFrame(uint8_t *yuv_data, uint32_t &width, uint32_t &height,
                           uint32_t &size) {
  if (nullptr == decoder) {
    return -1;
  }
  cudaVideoSurfaceFormat format = decoder->GetOutputFormat();
  if (format == cudaVideoSurfaceFormat_NV12) {
    uint8_t *data = nullptr;
    data = decoder->GetFrame();
    if (data) {
      yuv_data = data;
      width = decoder->GetWidth();
      height = decoder->GetHeight();
      size = width * height * 3 / 2;
      return 0;

      return -1;
    }
    return -1;
  }
  return -1;
}