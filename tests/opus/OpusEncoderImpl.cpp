#include "OpusEncoderImpl.h"

#include <stdlib.h>
#include <unistd.h>

#include <cstring>

#include "OpusDecoderImpl.h"
#define MAX_PACKET_SIZE 3 * 1276

OpusEncoderImpl::OpusEncoderImpl(int sampleRate, int channel)
    : channel_num(channel), sample_rate(sampleRate) {
  int err;
  int applications[3] = {OPUS_APPLICATION_AUDIO, OPUS_APPLICATION_VOIP,
                         OPUS_APPLICATION_RESTRICTED_LOWDELAY};

  encoder = opus_encoder_create(sampleRate, channel_num, applications[1], &err);

  if (err != OPUS_OK || encoder == NULL) {
    printf("打开opus 编码器失败\n");
  }

  opus_encoder_ctl(encoder, OPUS_SET_VBR(0));  // 0:CBR, 1:VBR
  opus_encoder_ctl(encoder, OPUS_SET_VBR_CONSTRAINT(true));
  opus_encoder_ctl(encoder, OPUS_SET_BITRATE(96000));
  opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(8));  // 8    0~10
  opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
  opus_encoder_ctl(encoder,
                   OPUS_SET_LSB_DEPTH(16));  // 每个采样16个bit，2个byte
  opus_encoder_ctl(encoder, OPUS_SET_DTX(0));
  opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(0));

  EncodeRun();
}

// every pcm frame takes 23ms
void OpusEncoderImpl::Feed(unsigned char *data, int len) {
  mutex.lock();
  for (auto i = 0; i < len; i++) {
    pcm_queue.emplace(data[i]);
  }
  mutex.unlock();
}

bool OpusEncoderImpl::PopFrame(StreamInfo &info) {
  if (info_queue.size() > 0) {
    access_mutex.lock();
    info = info_queue.front();
    info_queue.pop();
    access_mutex.unlock();
    return true;
  }

  return false;
}

// 48000 sample rate，48 samples/ms * 20ms * 2 channel = 1920
void OpusEncoderImpl::EncodeRun() {
  m_thread = std::make_unique<std::thread>([this]() {
    const int frame_size = 48 * 20;  // 960
    const int input_len = sizeof(opus_int16) * frame_size * 2;

    OpusDecoderImpl decoder(48000, channel_num);

    opus_int16 input_data[frame_size * 2] = {0};
    unsigned char input_buffer[input_len] = {0};
    unsigned char out_data[MAX_PACKET_SIZE] = {0};

    while (isRuning) {
      if (pcm_queue.size() >= input_len) {
        mutex.lock();
        for (int i = 0; i < input_len; i++) {
          input_buffer[i] = pcm_queue.front();
          pcm_queue.pop();
        }

        mutex.unlock();

        auto ret = opus_encode(encoder, (opus_int16 *)input_buffer, frame_size,
                               out_data, MAX_PACKET_SIZE);
        if (ret < 0) {
          printf("opus decode failed, %d\n", ret);
          break;
        }

        unsigned char *opus_buffer = (unsigned char *)malloc(ret);
        memcpy(opus_buffer, out_data, ret);
        decoder.Decode(opus_buffer, ret);

        StreamInfo info;
        info.data = opus_buffer;
        info.len = ret;
        info.dts = 20;
        access_mutex.lock();
        info_queue.push(info);
        access_mutex.unlock();

      } else {
        usleep(1000);
      }
    }
  });
}

void OpusEncoderImpl::Stop() {
  isRuning = false;
  m_thread->join();

  while (pcm_queue.size() > 0) {
    pcm_queue.pop();
  }

  opus_encoder_destroy(encoder);
}

OpusEncoderImpl::~OpusEncoderImpl() {}