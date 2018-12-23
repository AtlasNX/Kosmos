#pragma once

#include <vector>

#include <switch.h>

namespace Encoding {

  std::vector<u8> uft16leToUtf8(u8 *data, size_t length);
  std::vector<u8> uft16beToUtf8(u8 *data, size_t length);
  std::vector<u8> utf8ToUtf16le(u8 *data, size_t length);
  std::vector<u8> utf8ToUtf16be(u8 *data, size_t length);

}
