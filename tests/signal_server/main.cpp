#include "signal_server.h"

int main() {
    SignalServer s;
    // connect ws://localhost:9002
    s.run();
    return 0;
}