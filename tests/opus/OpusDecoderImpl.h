
#ifndef __OPUSDECODERIMPL_H
#define __OPUSDECODERIMPL_H
#include <stdio.h>

#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "base_type.h"
#include "opus/opus.h"

class OpusDecoderImpl {
 private:
  /* data */
  OpusDecoder *decoder;
  int sample_rate;
  int channel_num;
  FILE *pcm_file;

 public:
  bool Decode(unsigned char *in_data, int len);
  OpusDecoderImpl(int sampleRate, int channel);
  ~OpusDecoderImpl();
};

#endif