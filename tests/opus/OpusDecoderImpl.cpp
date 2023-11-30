#include "OpusDecoderImpl.h"
#define MAX_FRAME_SIZE 960
#define CHANNELS 1

OpusDecoderImpl::OpusDecoderImpl(int sampleRate, int channel) {
  int err;
  decoder = opus_decoder_create(sampleRate, channel, &err);
  opus_decoder_ctl(decoder, OPUS_SET_LSB_DEPTH(16));
  sample_rate = sample_rate;
  channel_num = channel;
  if (err < 0 || decoder == NULL) {
    printf("Create opus decoder failed\n");
    return;
  }

  pcm_file = fopen("decode.pcm", "wb+");
}

bool OpusDecoderImpl::Decode(unsigned char* in_data, int len) {
  unsigned char pcm_bytes[MAX_FRAME_SIZE * CHANNELS * 2];
  opus_int16 out[MAX_FRAME_SIZE * CHANNELS];
  auto frame_size = opus_decode(decoder, in_data, len, out, MAX_FRAME_SIZE, 0);

  if (frame_size < 0) {
    printf("Invalid frame size\n");
    return false;
  }

  for (auto i = 0; i < channel_num * frame_size; i++) {
    pcm_bytes[2 * i] = out[i] & 0xFF;
    pcm_bytes[2 * i + 1] = (out[i] >> 8) & 0xFF;
  }

  fwrite(pcm_bytes, sizeof(short), frame_size * channel_num, pcm_file);
  fflush(pcm_file);
  return true;
}

OpusDecoderImpl::~OpusDecoderImpl() {}