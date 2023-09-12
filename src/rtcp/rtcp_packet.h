#ifndef _RTCP_PACKET_H_
#define _RTCP_PACKET_H_

class RtcpPacket {
 public:
  typedef enum {
    UNKNOWN = 0,
    SR = 200,
    RR = 201,
    SDES = 202,
    BYE = 203,
    APP = 204
  } RTCP_TYPE;
};

#endif