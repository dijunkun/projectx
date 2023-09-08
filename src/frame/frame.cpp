#include "frame.h"

#include <string>

VideoFrame::VideoFrame() {}

VideoFrame::VideoFrame(const uint8_t *buffer, size_t size) {
  buffer_ = new uint8_t[size];
  memcpy(buffer_, buffer, size);
  size_ = size;
}

VideoFrame::VideoFrame(const VideoFrame &video_frame) {
  if (video_frame.size_ > 0) {
    buffer_ = new uint8_t[video_frame.size_];
    memcpy(buffer_, video_frame.buffer_, video_frame.size_);
    size_ = video_frame.size_;
  }
}

VideoFrame::VideoFrame(VideoFrame &&video_frame)
    : buffer_((uint8_t *)std::move(video_frame.buffer_)),
      size_(video_frame.size_) {
  video_frame.buffer_ = nullptr;
  video_frame.size_ = 0;
}

VideoFrame &VideoFrame::operator=(const VideoFrame &video_frame) {
  if (&video_frame != this) {
    if (buffer_) {
      delete buffer_;
      buffer_ = nullptr;
    }
    buffer_ = new uint8_t[video_frame.size_];
    memcpy(buffer_, video_frame.buffer_, video_frame.size_);
    size_ = video_frame.size_;
  }
  return *this;
}

VideoFrame &VideoFrame::operator=(VideoFrame &&video_frame) {
  if (&video_frame != this) {
    buffer_ = std::move(video_frame.buffer_);
    video_frame.buffer_ = nullptr;
    size_ = video_frame.size_;
    video_frame.size_ = 0;
  }
  return *this;
}

VideoFrame::~VideoFrame() {
  if (buffer_) {
    delete buffer_;
    buffer_ = nullptr;
  }

  size_ = 0;
}