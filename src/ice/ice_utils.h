/*
 * @Author: DI JUNKUN
 * @Date: 2026-09-07
 * Copyright (c) 2026 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _ICE_UTILS_H_
#define _ICE_UTILS_H_

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace minirtc {

inline constexpr char kRelayUpgradeAttribute[] = "a=x-minirtc-relay-upgrade:1";
inline constexpr char kP2pEnhancementAttribute[] =
    "a=x-minirtc-p2p-enhancement:1";

inline bool HasIceAttribute(const std::string& sdp,
                            const std::string& attribute) {
  std::istringstream lines(sdp);
  std::string line;
  while (std::getline(lines, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line == attribute) return true;
  }
  return false;
}

inline bool SupportsRelayUpgrade(const std::string& sdp) {
  return HasIceAttribute(sdp, kRelayUpgradeAttribute);
}

inline bool SupportsP2pEnhancement(const std::string& sdp) {
  return HasIceAttribute(sdp, kP2pEnhancementAttribute);
}

inline std::string TrimIceWhitespace(const std::string& value) {
  const auto start = value.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return {};
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(start, end - start + 1);
}

inline std::string GetIceUsername(const std::string& sdp) {
  const std::string prefix = "a=ice-ufrag:";
  std::istringstream lines(sdp);
  std::string line;
  while (std::getline(lines, line)) {
    if (line.compare(0, prefix.size(), prefix) == 0) {
      return TrimIceWhitespace(line.substr(prefix.size()));
    }
  }
  return {};
}

struct StunEndpoint {
  std::string host;
  uint16_t port = 0;
  std::string ToString() const {
    return (host.find(':') == std::string::npos ? host : "[" + host + "]") +
           ":" + std::to_string(port);
  }
};

struct IceCandidateSignal {
  std::string sdp;
  std::string ufrag;
  bool complete() const { return sdp.empty(); }
};

// Accept one candidate or one completion marker, never arbitrary SDP or a
// substring that merely happens to contain "end-of-candidates".
inline std::optional<IceCandidateSignal> ParseIceCandidateSignal(
    const std::string& candidate_sdp, const std::string& ufrag = {}) {
  if (candidate_sdp.size() > 4096 || ufrag.size() > 256 ||
      candidate_sdp.find('\0') != std::string::npos ||
      ufrag.find('\0') != std::string::npos ||
      ufrag.find_first_of(" \t\r\n") != std::string::npos)
    return std::nullopt;
  std::string sdp = TrimIceWhitespace(candidate_sdp);
  if (sdp.empty() || sdp == "a=end-of-candidates" || sdp == "end-of-candidates")
    return IceCandidateSignal{{}, ufrag};
  if (sdp.find_first_of("\r\n") != std::string::npos) return std::nullopt;
  if (sdp.compare(0, 2, "a=") == 0) sdp.erase(0, 2);
  if (sdp.compare(0, 10, "candidate:") != 0) return std::nullopt;

  IceCandidateSignal signal{"a=" + sdp, ufrag};
  std::istringstream fields(sdp);
  std::string token;
  // candidate foundation, component, transport, priority, address, port,
  // "typ", and candidate type precede all optional extension pairs.
  for (int i = 0; i < 8; ++i) {
    if (!(fields >> token)) return std::nullopt;
  }
  std::string value;
  while (fields >> token) {
    if (!(fields >> value)) return std::nullopt;
    if (token == "ufrag") {
      if (value.size() > 256 ||
          (!signal.ufrag.empty() && signal.ufrag != value))
        return std::nullopt;
      signal.ufrag = value;
    }
  }
  return signal;
}

}  // namespace minirtc

#endif
