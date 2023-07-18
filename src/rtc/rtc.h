#ifndef _RTC_H_
#define _RTC_H_

enum ws_status { WS_CONNECTING = 0, WS_OPEN, WS_FAILED, WS_CLOSED, WS_UNKNOWN };

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*OnReceiveBuffer)(unsigned char*, size_t, const char*,
                                const size_t);

typedef void (*NetStatusReport)(const unsigned short, const unsigned short);

typedef struct {
  const char* cfg_path;
  OnReceiveBuffer on_receive_buffer;
  NetStatusReport net_status_report;
} Params;

int CreatePeerConnection(Params params);

int CreatePeerConnectionWithID(Params params, const char* id);

int SendData(const char* data, size_t size);

int rtc();

int RegisterPeer();

#ifdef __cplusplus
}
#endif

#endif