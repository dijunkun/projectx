#ifndef _FRAME_H_
#define _FRAME_H_

#include <stdint.h>

class VideoFrame {
 public:
  VideoFrame();
  VideoFrame(const uint8_t *buffer, size_t size);
  VideoFrame(const uint8_t *buffer, size_t size, size_t width, size_t height);
  VideoFrame(const VideoFrame &video_frame);
  VideoFrame(VideoFrame &&video_frame);
  VideoFrame &operator=(const VideoFrame &video_frame);
  VideoFrame &operator=(VideoFrame &&video_frame);

  ~VideoFrame();

 public:
  const uint8_t *Buffer() { return buffer_; }
  const size_t Size() { return size_; }

 private:
  size_t width_ = 0;
  size_t height_ = 0;
  uint8_t *buffer_ = nullptr;
  size_t size_ = 0;
};

#endif