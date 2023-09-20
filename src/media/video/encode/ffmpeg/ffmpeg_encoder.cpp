#include "ffmpeg_encoder.h"

#include <chrono>

#include "log.h"

#define SAVE_ENCODER_STREAM 0

VideoEncoder::VideoEncoder() {
  if (SAVE_ENCODER_STREAM) {
    file_ = fopen("encode_stream.h264", "w+b");
    if (!file_) {
      LOG_WARN("Fail to open stream.h264");
    }
  }
}
VideoEncoder::~VideoEncoder() {
  if (SAVE_ENCODER_STREAM && file_) {
    fflush(file_);
    fclose(file_);
    file_ = nullptr;
  }

  av_packet_free(&packet_);

  if (nv12_data_) {
    free(nv12_data_);
    nv12_data_ = nullptr;
  }
}

int VideoEncoder::Init() {
  av_log_set_level(AV_LOG_ERROR);

  codec_ = avcodec_find_encoder(AV_CODEC_ID_H264);

  if (!codec_) {
    LOG_ERROR("Failed to find H.264 encoder");
    return -1;
  }

  codec_ctx_ = avcodec_alloc_context3(codec_);
  if (!codec_ctx_) {
    LOG_ERROR("Failed to allocate codec context");
    return -1;
  }

  codec_ctx_->codec_id = AV_CODEC_ID_H264;
  codec_ctx_->codec_type = AVMEDIA_TYPE_VIDEO;
  codec_ctx_->width = frame_width_;
  codec_ctx_->height = frame_height;
  codec_ctx_->time_base.num = 1;
  codec_ctx_->time_base.den = fps_;
  codec_ctx_->pix_fmt = AV_PIX_FMT_NV12;
  codec_ctx_->gop_size = keyFrameInterval_;
  codec_ctx_->keyint_min = keyFrameInterval_;
  codec_ctx_->max_b_frames = 0;
  codec_ctx_->bit_rate = maxBitrate_ * 500;

  // av_opt_set_int(codec_ctx_->priv_data, "qp", 51, 0);
  // av_opt_set_int(codec_ctx_->priv_data, "crf", 23, 0);

  av_opt_set(codec_ctx_->priv_data, "profile", "baseline", 0);
  av_opt_set(codec_ctx_->priv_data, "preset", "ultrafast", 0);
  av_opt_set(codec_ctx_->priv_data, "tune", "zerolatency", 0);

  if (avcodec_open2(codec_ctx_, codec_, nullptr) < 0) {
    LOG_ERROR("Failed to open codec");
    return -1;
  }

  frame_ = av_frame_alloc();
  frame_->format = codec_ctx_->pix_fmt;
  frame_->width = codec_ctx_->width;
  frame_->height = codec_ctx_->height;

  int ret = av_frame_get_buffer(frame_, 0);
  if (ret < 0) {
    LOG_ERROR("Could not allocate the raw frame");
    return -1;
  }

  packet_ = av_packet_alloc();

  return 0;
}

int VideoEncoder::Encode(
    const uint8_t *pData, int nSize,
    std::function<int(char *encoded_packets, size_t size)> on_encoded_image) {
  if (!codec_ctx_) {
    LOG_ERROR("Invalid codec context");
    return -1;
  }

  memcpy(frame_->data[0], pData, frame_->width * frame_->height);
  memcpy(frame_->data[1], pData + frame_->width * frame_->height,
         frame_->width * frame_->height / 2);

  frame_->pts = pts_++;

  int ret = avcodec_send_frame(codec_ctx_, frame_);

  // frame_->pict_type = AV_PICTURE_TYPE_I;
  while (ret >= 0) {
    ret = avcodec_receive_packet(codec_ctx_, packet_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0;
    } else if (ret < 0) {
      return -1;
    }

    // Remove first 6 bytes in I frame, SEI ?
    if (0x00 == packet_->data[0] && 0x00 == packet_->data[1] &&
        0x00 == packet_->data[2] && 0x01 == packet_->data[3] &&
        0x09 == packet_->data[4] && 0x10 == packet_->data[5]) {
      packet_->data += 6;
      packet_->size -= 6;
    }

    if (on_encoded_image) {
      on_encoded_image((char *)packet_->data, packet_->size);
      if (SAVE_ENCODER_STREAM) {
        fwrite(packet_->data, 1, packet_->size, file_);
      }
    } else {
      OnEncodedImage((char *)packet_->data, packet_->size);
    }
    av_packet_unref(packet_);
  }

  return 0;
}

int VideoEncoder::OnEncodedImage(char *encoded_packets, size_t size) {
  LOG_INFO("OnEncodedImage not implemented");
  return 0;
}

void VideoEncoder::ForceIdr() {}
