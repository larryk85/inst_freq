#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <windows.h>
#include <winnt.h>

inline static std::vector<uint8_t*> load_binary_impl(std::string_view path) {
   std::vector<uint8_t> bytes;
   HANDLE f = CreateFileA(path.data(),
                          GENERIC_READ, FILE_SHARE_READ,
                          nullptr, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, nullptr);

   if (f == INVALID_HANDLE_VALUE)
      return bytes;

   HANDLE map = CreateFileMapping(f, nullptr, PAGE_READONLY, 0, 0, nullptr);

   if (!map) {
      CloseHandle(f);
      return bytes;
   }

   uint8_t* base = (uint8_t*)MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);

   if (!base) {
      CloseHandle(map);
      CloseHandle(f);
      return bytes;
   }

   auto dos = (IMAGE_DOS_HEADER*)base;
   if (dos->e_magic != IMAGE_DOS_SIGNATURE)
      return bytes;

   auto nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
   if (nt->Signature != IMAGE_NT_SIGNATURE)
      return bytes;

   IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);

   for (uint32_t i=0; i < nt->FileHeader.NumberOfSections; i++) {
      if (std::memcmp(sec[i].Name, ".text", 5) == 0) {
         uint8_t* ptr = base + sec[i].PointerToRawData;
         bytes.resize(sec[i].SizeOfRawData);
         std::memcpy(bytes.data(), ptr, bytes.size());
         UnmapViewOfFile(base);
         CloseHandle(map);
         CloseHandle(f);
         return bytes;
      }
   }

   UnmapViewOfFile(base);
   CloseHandle(map);
   CloseHandle(f);
   return bytes;
}