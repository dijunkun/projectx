
#ifndef __OPUSENCODERIMPL_H
#define __OPUSENCODERIMPL_H
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "OpusDecoderImpl.h"
#include "base_type.h"
#include "opus/opus.h"

class OpusEncoderImpl {
 private:
  OpusEncoder *encoder;
  const int channel_num;
  int sample_rate;
  std::queue<StreamInfo> info_queue;
  std::queue<unsigned char> pcm_queue;
  std::mutex mutex;
  bool isRuning = true;
  std::mutex access_mutex;
  std::unique_ptr<std::thread> m_thread;

  OpusDecoderImpl *decoder = nullptr;

 public:
  OpusEncoderImpl(int sampleRate, int channel);
  void Feed(unsigned char *data, int len);
  bool PopFrame(StreamInfo &info);
  void EncodeRun();
  void Stop();
  ~OpusEncoderImpl();
};

#endif