#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>

int CommonDummy();

constexpr size_t HASH_STRING_PIECE(const char *string_piece) {
  std::size_t result = 0;
  while (*string_piece) {
    result = (result * 131) + *string_piece++;
  }
  return result;
}

constexpr size_t operator"" _H(const char *string_piece, size_t) {
  return HASH_STRING_PIECE(string_piece);
}

inline const std::string GetIceUsername(const std::string &sdp) {
  std::string result = "";

  std::string start = "ice-ufrag:";
  std::string end = "\r\n";
  size_t startPos = sdp.find(start);
  size_t endPos = sdp.find(end);

  if (startPos != std::string::npos && endPos != std::string::npos) {
    result = sdp.substr(startPos + start.length(),
                        endPos - startPos - start.length());
  }
  return result;
}

#endif