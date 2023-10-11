#ifndef _X_H_
#define _X_H_

#include <stdint.h>
#include <stdlib.h>

enum ws_status { WS_CONNECTING = 0, WS_OPEN, WS_FAILED, WS_CLOSED, WS_UNKNOWN };
enum DATA_TYPE { VIDEO = 0, AUDIO, DATA };

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Peer PeerPtr;

typedef void (*OnReceiveBuffer)(const char*, size_t, const char*, size_t);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char* cfg_path;
  OnReceiveBuffer on_receive_video_buffer;
  OnReceiveBuffer on_receive_audio_buffer;
  OnReceiveBuffer on_receive_data_buffer;
  NetStatusReport net_status_report;
} Params;

PeerPtr* CreatePeer(const Params* params);

int Init(PeerPtr* peer_ptr, const char* user_id);

int CreateConnection(PeerPtr* peer_ptr, const char* transmission_id,
                     const char* password);

int JoinConnection(PeerPtr* peer_ptr, const char* transmission_id,
                   const char* password);

int LeaveConnection(PeerPtr* peer_ptr);

int SendData(PeerPtr* peer_ptr, DATA_TYPE data_type, const char* data,
             size_t size);

#ifdef __cplusplus
}
#endif

#endif