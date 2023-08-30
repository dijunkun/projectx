#include "nv_encoder.h"

#include <chrono>

#include "log.h"

#define SAVE_ENCODER_STREAM 0

VideoEncoder::VideoEncoder() {
  if (SAVE_ENCODER_STREAM) {
    file_ = fopen("saved/stream.h264", "w+b");
    if (!file_) {
      LOG_WARN("Fail to open saved/stream.h264");
    }
  }
}
VideoEncoder::~VideoEncoder() {
  if (SAVE_ENCODER_STREAM && file_) {
    fflush(file_);
    fclose(file_);
    file_ = nullptr;
  }

  if (nv12_data_) {
    free(nv12_data_);
    nv12_data_ = nullptr;
  }
}

int VideoEncoder::Init() {
  // Init cuda context
  int num_of_GPUs = 0;
  CUdevice cuda_device;
  bool cuda_ctx_succeed =
      (index_of_GPU >= 0 && cuInit(0) == CUresult::CUDA_SUCCESS &&
       cuDeviceGetCount(&num_of_GPUs) == CUresult::CUDA_SUCCESS &&
       (num_of_GPUs > 0 && index_of_GPU < num_of_GPUs) &&
       cuDeviceGet(&cuda_device, index_of_GPU) == CUresult::CUDA_SUCCESS &&
       cuCtxCreate(&cuda_context_, 0, cuda_device) == CUresult::CUDA_SUCCESS);
  if (!cuda_ctx_succeed) {
  }

  encoder_ = new NvEncoderCuda(cuda_context_, frame_width, frame_height,
                               NV_ENC_BUFFER_FORMAT::NV_ENC_BUFFER_FORMAT_NV12);

  // Init encoder_ session
  NV_ENC_INITIALIZE_PARAMS init_params;
  init_params.version = NV_ENC_INITIALIZE_PARAMS_VER;
  NV_ENC_CONFIG encode_config = {NV_ENC_CONFIG_VER};
  init_params.encodeConfig = &encode_config;

  encoder_->CreateDefaultEncoderParams(&init_params, codec_guid, preset_guid,
                                       tuning_info);

  init_params.encodeWidth = frame_width;
  init_params.encodeHeight = frame_height;
  init_params.encodeConfig->profileGUID = NV_ENC_H264_PROFILE_BASELINE_GUID;
  init_params.encodeConfig->encodeCodecConfig.h264Config.level =
      NV_ENC_LEVEL::NV_ENC_LEVEL_H264_31;
  // TO TEST: not tested yet
  // init_params.encodeConfig->gopLength = NVENC_INFINITE_GOPLENGTH;
  init_params.encodeConfig->gopLength = keyFrameInterval_;
  // Donot use B-frame for realtime application
  init_params.encodeConfig->frameIntervalP = 1;
  init_params.encodeConfig->rcParams.rateControlMode =
      NV_ENC_PARAMS_RC_MODE::NV_ENC_PARAMS_RC_CBR;
  init_params.encodeConfig->rcParams.maxBitRate = maxBitrate_ * 1000;
  init_params.encodeConfig->encodeCodecConfig.h264Config.sliceMode = 1;
  init_params.encodeConfig->encodeCodecConfig.h264Config.sliceModeData =
      max_payload_size_;

  encoder_->CreateEncoder(&init_params);
  return 0;
}

int VideoEncoder::Encode(const uint8_t *pData, int nSize) {
  if (!encoder_) {
    LOG_ERROR("Invalid encoder");
    return -1;
  }

  if (0 == seq_++ % (30 * 5)) {
    ForceIdr();
  }

#ifdef SHOW_SUBMODULE_TIME_COST
  auto start = std::chrono::steady_clock::now();
#endif

  const NvEncInputFrame *encoder_inputframe = encoder_->GetNextInputFrame();

  NvEncoderCuda::CopyToDeviceFrame(
      cuda_context_,
      (void *)pData,  // NOLINT
      0, (CUdeviceptr)encoder_inputframe->inputPtr, encoder_inputframe->pitch,
      encoder_->GetEncodeWidth(), encoder_->GetEncodeHeight(),
      CU_MEMORYTYPE_HOST, encoder_inputframe->bufferFormat,
      encoder_inputframe->chromaOffsets, encoder_inputframe->numChromaPlanes);

  encoder_->EncodeFrame(encoded_packets_);

  if (encoded_packets_.size() < 1) {
    LOG_WARN("empty encoded_packets_");
    return -1;
  }

  for (const auto &packet : encoded_packets_) {
    OnEncodedImage((char *)packet.data(), packet.size());

    if (SAVE_ENCODER_STREAM) {
      fwrite((unsigned char *)packet.data(), 1, packet.size(), file_);
    }
  }

#ifdef SHOW_SUBMODULE_TIME_COST
  auto encode_time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now() - start)
                              .count();
  LOG_INFO("Encode time cost {}ms", encode_time_cost);
#endif

  return 0;
}

int VideoEncoder::OnEncodedImage(char *encoded_packets, size_t size) {
  LOG_INFO("output encoded image");
  fwrite(encoded_packets, 1, size, file_);
  return 0;
}

void VideoEncoder::ForceIdr() {
  NV_ENC_RECONFIGURE_PARAMS reconfig_params;
  reconfig_params.version = NV_ENC_RECONFIGURE_PARAMS_VER;

  NV_ENC_INITIALIZE_PARAMS init_params;
  NV_ENC_CONFIG encode_config = {NV_ENC_CONFIG_VER};
  init_params.encodeConfig = &encode_config;
  encoder_->GetInitializeParams(&init_params);

  reconfig_params.reInitEncodeParams = init_params;
  reconfig_params.forceIDR = 1;
  reconfig_params.resetEncoder = 1;

  encoder_->Reconfigure(&reconfig_params);
}
