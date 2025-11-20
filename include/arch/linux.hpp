#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <libelf.h>
#include <gelf.h>
#include <cstring>

std::vector<uint8_t> load_binary_impl(std::string_view path) {
   std::vector<uint8_t> bytes;
   if (elf_version(EV_CURRENT) == EV_NONE)
      return bytes;

   FILE* f = fopen(path.data(), "rb");

   if (!f) 
      return bytes;

   auto elf = elf_begin(fileno(f), ELF_C_READ, nullptr);

   if (!elf) {
      fclose(f);
      return bytes;
   }

   std::size_t shstrndx;
   if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
      elf_end(elf);
      fclose(f);
      return bytes;
   }

   Elf_Scn* scn = nullptr;
   while ((scn = elf_nextscn(elf, scn)) != nullptr) {
      GElf_Shdr shdr;
      if (!gelf_getshdr(scn, &shdr))
         continue;

      auto name = elf_strptr(elf, shstrndx, shdr.sh_name);
      if (!name) 
         continue;

      if (strcmp(name, ".text") == 0) {
         Elf_Data* data = elf_getdata(scn, nullptr);
         if (data) {
            uint8_t* raw_bytes = (uint8_t*)data->d_buf;
            bytes.resize(data->d_size);
            std::memcpy(bytes.data(), raw_bytes, data->d_size);
            elf_end(elf);
            fclose(f);
            return bytes;
         }
      }
   }

   elf_end(elf);
   fclose(f);
   return bytes;
}