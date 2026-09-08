/*
 * @Author: DI JUNKUN
 * @Date: 2026-09-08
 * Copyright (c) 2026 by DI JUNKUN, All Rights Reserved.
 */

#ifndef _NAT_TRAVERSAL_H_
#define _NAT_TRAVERSAL_H_

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace minirtc {

struct NatMappingSample {
  std::string server_ip;
  uint16_t server_port;
  std::string mapped_ip;
  uint16_t mapped_port;
  uint32_t sequence;
};

struct NatMappingAnalysis {
  // These describe observed mappings, never NAT filtering or a guarantee
  // about future allocations. Two samples cannot establish a linear pattern.
  std::string mapping = "insufficient-samples";
  std::string port_pattern = "unknown";
  int port_step = 0;
  size_t samples = 0;
};

inline NatMappingAnalysis AnalyzeNatMappings(
    std::vector<NatMappingSample> samples) {
  NatMappingAnalysis result;
  std::set<std::pair<std::string, uint16_t>> endpoints;
  samples.erase(
      std::remove_if(
          samples.begin(), samples.end(),
          [&](const auto& s) {
            return !s.server_port || !s.mapped_port || s.server_ip.empty() ||
                   s.mapped_ip.empty() ||
                   !endpoints.emplace(s.server_ip, s.server_port).second;
          }),
      samples.end());
  result.samples = samples.size();
  if (samples.size() < 2) return result;
  std::sort(samples.begin(), samples.end(), [](const auto& a, const auto& b) {
    return a.sequence < b.sequence;
  });
  const auto& first = samples.front();
  bool stable = true, same_ip = true, different_servers = false;
  for (const auto& s : samples) {
    same_ip &= s.mapped_ip == first.mapped_ip;
    stable &=
        s.mapped_ip == first.mapped_ip && s.mapped_port == first.mapped_port;
    different_servers |= s.server_ip != first.server_ip;
  }
  result.mapping =
      stable ? (different_servers ? "endpoint-independent"
                                  : "insufficient-destination-diversity")
             : "destination-dependent";
  result.port_pattern = stable ? "stable" : "unknown";
  if (stable || !same_ip || samples.size() < 3) return result;
  const int step = int(samples[1].mapped_port) - int(first.mapped_port);
  bool linear = step != 0 && step >= -1024 && step <= 1024;
  for (size_t i = 1; i < samples.size(); ++i) {
    linear &=
        samples[i].sequence > samples[i - 1].sequence &&
        int(samples[i].mapped_port) - int(samples[i - 1].mapped_port) == step;
  }
  result.port_pattern = linear ? "linear-observed" : "irregular-observed";
  result.port_step = linear ? step : 0;
  return result;
}

inline bool IsPublicIpv4ForPrediction(const std::string& ip) {
  unsigned bytes[4] = {};
  size_t i = 0;
  for (unsigned& byte : bytes) {
    const size_t start = i;
    while (i < ip.size() && ip[i] >= '0' && ip[i] <= '9') {
      byte = byte * 10 + unsigned(ip[i++] - '0');
      if (byte > 255 || i - start > 3) return false;
    }
    if (i == start) return false;
    if (&byte != &bytes[3]) {
      if (i == ip.size() || ip[i++] != '.') return false;
    }
  }
  if (i != ip.size()) return false;
  return bytes[0] != 0 && bytes[0] != 10 && bytes[0] != 127 && bytes[0] < 224 &&
         !(bytes[0] == 100 && bytes[1] >= 64 && bytes[1] <= 127) &&
         !(bytes[0] == 169 && bytes[1] == 254) &&
         !(bytes[0] == 172 && bytes[1] >= 16 && bytes[1] <= 31) &&
         !(bytes[0] == 192 &&
           (bytes[1] == 168 || (bytes[1] == 0 && bytes[2] == 0))) &&
         !(bytes[0] == 198 && (bytes[1] == 18 || bytes[1] == 19));
}

class RemotePortPredictor {
 public:
  static constexpr size_t kMaxPredictions = 32;
  // group identifies the peer's public IP and original local IP:port.
  // Generated ports are hypotheses. Only authenticated ICE checks can select
  // them.
  std::vector<uint16_t> Observe(const std::string& group, uint16_t port) {
    if (!port || (groups_.count(group) == 0 && groups_.size() >= 8)) return {};
    auto& state = groups_[group];
    if (std::find(state.observed.begin(), state.observed.end(), port) !=
            state.observed.end() ||
        state.observed.size() >= 8)
      return {};
    state.observed.push_back(port);
    if (state.observed.size() < 2 || predictions_ >= kMaxPredictions) return {};
    int step = 0;
    const auto& observed = state.observed;
    if (observed.size() >= 3) {
      step = int(observed[1]) - int(observed[0]);
      for (size_t i = 2; i < observed.size(); ++i)
        if (int(observed[i]) - int(observed[i - 1]) != step) {
          step = 0;
          break;
        }
      if (step < -64 || step > 64) step = 0;
    }
    std::vector<uint16_t> result;
    for (int i = 1; i <= 8 && predictions_ < kMaxPredictions; ++i) {
      const int offset = step ? step * i : ((i + 1) / 2) * (i % 2 ? 1 : -1);
      const int candidate = int(port) + offset;
      if (candidate < 1024 || candidate > 65535 ||
          std::find(observed.begin(), observed.end(), candidate) !=
              observed.end() ||
          !state.predicted.insert(static_cast<uint16_t>(candidate)).second)
        continue;
      result.push_back(static_cast<uint16_t>(candidate));
      ++predictions_;
    }
    return result;
  }
  size_t predictions() const { return predictions_; }

 private:
  struct Group {
    std::vector<uint16_t> observed;
    std::set<uint16_t> predicted;
  };
  std::map<std::string, Group> groups_;
  size_t predictions_ = 0;
};

}  // namespace minirtc

#endif
