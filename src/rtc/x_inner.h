#ifndef _X_INNER_H_
#define _X_INNER_H_

#include "peer_connection.h"

struct Peer {
  PeerConnection *peer_connection;
  PeerConnectionParams pc_params;
};

#endif