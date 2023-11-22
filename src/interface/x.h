#ifndef _X_H_
#define _X_H_

#ifdef DLL_EXPORTS
#define DLLAPI __declspec(dllexport)
#else
#define DLLAPI __declspec(dllimport)
#endif

#include <stdint.h>
#include <stdlib.h>

enum DATA_TYPE { VIDEO = 0, AUDIO, DATA };
enum ConnectionStatus {
  Connecting = 0,
  Connected,
  Failed,
  Closed,
  IncorrectPassword
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Peer PeerPtr;

typedef void (*OnReceiveBuffer)(const char*, size_t, const char*, size_t);

typedef void (*OnConnectionStatus)(ConnectionStatus status);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char* cfg_path;
  OnReceiveBuffer on_receive_video_buffer;
  OnReceiveBuffer on_receive_audio_buffer;
  OnReceiveBuffer on_receive_data_buffer;
  OnConnectionStatus on_connection_status;
  NetStatusReport net_status_report;
} Params;

DLLAPI PeerPtr* CreatePeer(const Params* params);

DLLAPI int Init(PeerPtr* peer_ptr, const char* user_id);

DLLAPI int CreateConnection(PeerPtr* peer_ptr, const char* transmission_id,
                            const char* password);

DLLAPI int JoinConnection(PeerPtr* peer_ptr, const char* transmission_id,
                          const char* password);

DLLAPI int LeaveConnection(PeerPtr* peer_ptr);

DLLAPI int SendData(PeerPtr* peer_ptr, DATA_TYPE data_type, const char* data,
                    size_t size);

#ifdef __cplusplus
}
#endif

#endif