#ifndef _X_H_
#define _X_H_

#include <stdint.h>
#include <stdlib.h>

enum ws_status { WS_CONNECTING = 0, WS_OPEN, WS_FAILED, WS_CLOSED, WS_UNKNOWN };

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Peer PeerPtr;

typedef void (*OnReceiveBuffer)(unsigned char*, size_t, const char*,
                                const size_t);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char* cfg_path;
  OnReceiveBuffer on_receive_buffer;
  NetStatusReport net_status_report;
} Params;

PeerPtr* CreatePeer(const Params* params);

int CreateConnection(PeerPtr* peer_ptr, const char* transmission_id,
                     const char* user_id);

int JoinConnection(PeerPtr* peer_ptr, const char* transmission_id,
                   const char* user_id);

int SendData(PeerPtr* peer_ptr, const char* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif