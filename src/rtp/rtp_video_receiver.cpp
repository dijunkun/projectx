#include "rtp_video_receiver.h"

#include "log.h"

#define NV12_BUFFER_SIZE (1280 * 720 * 3 / 2)

RtpVideoReceiver::RtpVideoReceiver() {}

RtpVideoReceiver::~RtpVideoReceiver() {
  if (jitter_thread_ && jitter_thread_->joinable()) {
    jitter_thread_->join();
    delete jitter_thread_;
    jitter_thread_ = nullptr;
  }
}

void RtpVideoReceiver::InsertRtpPacket(RtpPacket& rtp_packet) {
  if (!jitter_thread_) {
    jitter_thread_ = new std::thread(&RtpVideoReceiver::Process, this);
  }

  if (NAL_UNIT_TYPE::NALU == rtp_packet.NalUnitType()) {
    compelete_video_frame_queue_.push(
        VideoFrame(rtp_packet.Payload(), rtp_packet.Size()));
    // if (on_receive_complete_frame_) {
    //   auto now_complete_frame_ts =
    //       std::chrono::high_resolution_clock::now().time_since_epoch().count()
    //       / 1000000;
    //   uint32_t duration = now_complete_frame_ts - last_complete_frame_ts_;
    //   LOG_ERROR("Duration {}", 1000 / duration);
    //   last_complete_frame_ts_ = now_complete_frame_ts;

    //   on_receive_complete_frame_(
    //       VideoFrame(rtp_packet.Payload(), rtp_packet.Size()));
    // }
  } else if (NAL_UNIT_TYPE::FU_A == rtp_packet.NalUnitType()) {
    incomplete_frame_list_[rtp_packet.SequenceNumber()] = rtp_packet;
    bool complete = CheckIsFrameCompleted(rtp_packet);
  }
}

bool RtpVideoReceiver::CheckIsFrameCompleted(RtpPacket& rtp_packet) {
  if (rtp_packet.FuAEnd()) {
    size_t complete_frame_size = 0;
    uint16_t end_seq = rtp_packet.SequenceNumber();
    if (incomplete_frame_list_.size() == end_seq) {
      return true;
    }

    while (end_seq--) {
      auto it = incomplete_frame_list_.find(end_seq);
      complete_frame_size += it->second.PayloadSize();
      if (it == incomplete_frame_list_.end()) {
        return false;
      } else if (!it->second.FuAStart()) {
        continue;
      } else if (it->second.FuAStart()) {
        if (!nv12_data_) {
          nv12_data_ = new uint8_t[NV12_BUFFER_SIZE];
        }

        size_t complete_frame_size = 0;
        for (size_t start = it->first; start <= rtp_packet.SequenceNumber();
             start++) {
          memcpy(nv12_data_ + complete_frame_size,
                 incomplete_frame_list_[start].Payload(),
                 incomplete_frame_list_[start].PayloadSize());

          complete_frame_size += incomplete_frame_list_[start].PayloadSize();
          incomplete_frame_list_.erase(start);
        }

        compelete_video_frame_queue_.push(
            VideoFrame(nv12_data_, complete_frame_size));

        // if (on_receive_complete_frame_) {
        //   auto now_complete_frame_ts =
        //   std::chrono::high_resolution_clock::now()
        //                                    .time_since_epoch()
        //                                    .count() /
        //                                1000000;
        //   uint32_t duration = now_complete_frame_ts -
        //   last_complete_frame_ts_; LOG_ERROR("Duration {}", 1000 / duration);
        //   last_complete_frame_ts_ = now_complete_frame_ts;

        //   on_receive_complete_frame_(
        //       VideoFrame(nv12_data_, complete_frame_size));
        // }
        return true;
      } else {
        LOG_WARN("What happened?")
        return false;
      }
    }

    return true;
  }
  return false;
}

void RtpVideoReceiver::Process() {
  while (1) {
    if (!compelete_video_frame_queue_.isEmpty()) {
      VideoFrame video_frame;
      compelete_video_frame_queue_.pop(video_frame);
      if (on_receive_complete_frame_) {
        auto now_complete_frame_ts = std::chrono::high_resolution_clock::now()
                                         .time_since_epoch()
                                         .count() /
                                     1000000;
        uint32_t duration = now_complete_frame_ts - last_complete_frame_ts_;
        LOG_ERROR("Duration {}", 1000 / duration);
        last_complete_frame_ts_ = now_complete_frame_ts;
        on_receive_complete_frame_(video_frame);
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(13));
  }
}