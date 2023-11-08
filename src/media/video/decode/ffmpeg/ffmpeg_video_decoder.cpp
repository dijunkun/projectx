#include "ffmpeg_video_decoder.h"

#include "log.h"

#define SAVE_DECODER_STREAM 0

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

FfmpegVideoDecoder::FfmpegVideoDecoder() {}

FfmpegVideoDecoder::~FfmpegVideoDecoder() {
  if (SAVE_DECODER_STREAM && file_) {
    fflush(file_);
    fclose(file_);
    file_ = nullptr;
  }

  if (decoded_frame_) {
    delete decoded_frame_;
    decoded_frame_ = nullptr;
  }

  if (packet_) {
    av_packet_free(&packet_);
  }

  if (frame_) {
    av_frame_free(&frame_);
  }
  if (frame_nv12_) {
    av_frame_free(&frame_nv12_);
  }

  if (img_convert_ctx) {
    sws_freeContext(img_convert_ctx);
  }
  if (codec_ctx_) {
    avcodec_close(codec_ctx_);
  }
  if (codec_ctx_) {
    av_free(codec_ctx_);
  }
}

int FfmpegVideoDecoder::Init() {
  av_log_set_level(AV_LOG_ERROR);

  codec_id_ = AV_CODEC_ID_H264;
  codec_ = avcodec_find_decoder(codec_id_);
  if (!codec_) {
    printf("Codec not found\n");
    return -1;
  }
  codec_ctx_ = avcodec_alloc_context3(codec_);
  if (!codec_ctx_) {
    printf("Could not allocate video codec context\n");
    return -1;
  } else {
    LOG_INFO("Use H264 decoder [{}]", codec_->name);
  }

  codec_ctx_->time_base.num = 1;
  codec_ctx_->frame_number = 1;
  codec_ctx_->codec_type = AVMEDIA_TYPE_VIDEO;
  codec_ctx_->bit_rate = 0;
  codec_ctx_->time_base.den = 29;
  codec_ctx_->width = 1280;
  codec_ctx_->height = 720;
  codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;  // yuv420 default?
  codec_ctx_->color_range = AVCOL_RANGE_MPEG;

  if (avcodec_open2(codec_ctx_, codec_, NULL) < 0) {
    printf("Could not open codec\n");
    return -1;
  }

  frame_ = av_frame_alloc();
  frame_nv12_ = av_frame_alloc();

  packet_ = av_packet_alloc();

  img_convert_ctx =
      sws_getContext(1280, 720, AV_PIX_FMT_YUV420P, 1280, 720, AV_PIX_FMT_NV12,
                     SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

  decoded_frame_ = new VideoFrame(1280 * 720 * 3 / 2);

  if (SAVE_DECODER_STREAM) {
    file_ = fopen("decode_stream.yuv", "w+b");
    if (!file_) {
      LOG_WARN("Fail to open stream.yuv");
    }
  }
  return 0;
}

int FfmpegVideoDecoder::Decode(
    const uint8_t *data, int size,
    std::function<void(VideoFrame)> on_receive_decoded_frame) {
  if (!first_) {
    if ((*(data + 4) & 0x1f) != 0x07) {
      return -1;
    } else {
      first_ = true;
    }
  }

  packet_->data = (uint8_t *)data;
  packet_->size = size;

  int ret = avcodec_send_packet(codec_ctx_, packet_);
  av_packet_unref(packet_);

  while (ret >= 0) {
    ret = avcodec_receive_frame(codec_ctx_, frame_);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      continue;
    } else if (ret < 0) {
      LOG_ERROR("Error receive decoding video frame ret=%d", ret);
      continue;
    }

    if (on_receive_decoded_frame) {
      uint64_t start_ts = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch())
              .count());

      if (1) {
        av_image_fill_arrays(frame_nv12_->data, frame_nv12_->linesize,
                             decoded_frame_->GetBuffer(), AV_PIX_FMT_NV12,
                             frame_->width, frame_->height, 1);

        sws_scale(img_convert_ctx, frame_->data, frame_->linesize, 0,
                  frame_->height, frame_nv12_->data, frame_nv12_->linesize);
      } else {
        memcpy(decoded_frame_->GetBuffer(), frame_->data[0],
               frame_->width * frame_->height);
        memcpy(decoded_frame_->GetBuffer() + frame_->width * frame_->height,
               frame_->data[1], frame_->width * frame_->height / 2);
      }

      uint64_t now_ts = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch())
              .count());

      // LOG_ERROR("cost {}", now_ts - start_ts);

      on_receive_decoded_frame(*decoded_frame_);
      if (SAVE_DECODER_STREAM) {
        fwrite((unsigned char *)decoded_frame_->Buffer(), 1,
               decoded_frame_->Size(), file_);
      }
    }
  }

  return 0;
}