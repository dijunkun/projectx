/*
 * @Author: DI JUNKUN
 * @Date: 2023-11-24
 * Copyright (c) 2023 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _AUDIO_ENCODER_H_
#define _AUDIO_ENCODER_H_

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "opus/opus.h"

class AudioEncoder {
 public:
  AudioEncoder(int sample_rate, int channel_num, int frame_size);
  ~AudioEncoder();

 public:
  int Init();
  int Encode(const uint8_t* data, int size,
             std::function<int(char* encoded_audio_buffer, size_t size)>
                 on_encoded_audio_buffer);
  int OnEncodedAudioBuffer(char* encoded_audio_buffer, size_t size);

 private:
  OpusEncoder* opus_encoder_ = nullptr;
  int sample_rate_ = 48000;
  int channel_num_ = 1;
  int frame_size_ = 480;

  std::queue<unsigned char> pcm_queue;
  std::function<int(char* encoded_audio_buffer, size_t size)>
      on_encoded_audio_buffer_ = nullptr;
};

#endif