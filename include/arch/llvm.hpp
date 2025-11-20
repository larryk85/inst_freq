#pragma once

#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/MachO.h>
#include <llvm/Object/COFF.h>
#include <llvm/Object/Binary.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Error.h>

inline static std::vector<uint8_t> load_binary_impl(std::string_view path) {
   using namespace llvm;
   using namespace llvm::object;

   std::vector<uint8_t> bytes;

   auto buf_or_err = MemoryBuffer::getFile(path);
   if (!buf_or_err) {
      std::cerr << "Error: cannot open file with LLVM\n";
      return bytes;
   }

   auto bin_or_err = createBinary(buf_or_err->get()->getMemBufferRef());
   if (!bin_or_err) {
      std::cerr << "Error: createBinary failed\n";
      return bytes;
   }

   Binary* bin = bin_or_err->get();
   if (auto* obj = dyn_cast<ObjectFile>(bin)) {
      for (auto& sec : obj->sections()) {
         if (auto name_or_err = sec.getName(); name_or_err->str() == ".text") {
            if (auto data = sec.getContents()) {
               bytes.assign(data->bytes_begin(), data->bytes_end());
               return bytes;
            }
         }
      }
   }
   return bytes;
}