#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

#if defined(USE_LLVM_LIBOBJECT)
#include "arch/llvm.hpp"
#elif defined(__linux__)
#include "arch/linux.hpp"
#elif defined(_WIN32)
#include "arch/win32.hpp"
#elif defined(__APPLE__)
#include "arch/mac.hpp"
#endif

inline static std::vector<uint8_t> load_binary(std::string_view path) {
   return load_binary_impl(path);
}