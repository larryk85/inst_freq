#pragma once

#include <cstdint>
#include <fstream>
#include <string_view>
#include <vector>

#include <mach-o/loader.h>
#include <mach-o/fat.h>

inline static std::vector<uint8_t> load_binary_impl(std::string_view path) {
   std::vector<uint8_t> bytes;
   std::ifstream f(path.data(), std::ios::binary);

   if (!f)
      return bytes;

   std::vector<uint8_t> buf((std::istreambuf_iterator<char>(f)),
                             std::istreambuf_iterator<char>());
   
   auto hdr = (mach_header_64*)buf.data();
   if (hdr->magic != MH_MAGIC_64)
      return bytes;

   uint8_t* p = buf.data() + sizeof(mach_header_64);

   for (uint32_t i=0; i < hdr->ncmds; i++) {
      auto cmd = (load_command*)p;
      if (cmd->cmd == LC_SEGMENT_64) {
         auto seg = (segment_command_64*)cmd;
         if (strcmp(seg->segname, "__TEXT") == 0) {
            auto sec = (section_64*)(seg+1);
            for (uint32_t j=0; j < seg->nsects; j++) {
               if (strcmp(sec[j].sectname, "__text") == 0) {
                  uint64_t offset = sec[j].offset;
                  uint64_t size   = sec[j].size;
                  bytes.resize(size);
                  std::memcpy(bytes.data(), buf.data()+offset, size);
                  return bytes;
               }
            }
         }
      }
      p += cmd->cmdsize;
   }

   return bytes;
}