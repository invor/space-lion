#ifndef _FBX_UNZIP_H
#define _FBX_UNZIP_H _FBX_UNZIP_H

#include "fbx_types.hpp"

namespace FBX {
	std::vector<uint8_t> inflate(const std::vector<uint8_t> &data, size_t out_hint = 0);
}

#endif
