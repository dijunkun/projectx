#ifndef _RTC_H_
#define _RTC_H_

enum ws_status { WS_CONNECTING = 0, WS_OPEN, WS_FAILED, WS_CLOSED, WS_UNKNOWN };

#ifdef __cplusplus
extern "C" {
#endif

int CreatePeerConnection(const char* uri);

int CreatePeerConnectionWithID(const char* uri, const char* id);

int SendData(const char* data, size_t size);

int rtc();

int RegisterPeer();

#ifdef __cplusplus
}
#endif

#endif