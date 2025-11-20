#include <Zydis/Zydis.h>


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <binary_loader.hpp>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input ELF> <output file>\n";
        if constexpr (USE_LLVM_LIBOBJECT)
            std::cerr << "Built with LLVM support." << std::endl;
        return 1;
    }

    const char* in  = argv[1];
    const char* out = argv[2];

    auto text = load_binary(in);
    if (text.empty()) {
        std::cerr << "Could not load .text section\n";
        return 1;
    }

    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder,
                     ZYDIS_MACHINE_MODE_LONG_64,
                     ZYDIS_STACK_WIDTH_64);

    std::map<std::string, size_t> freq;

    ZydisDecodedInstruction instr;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

    size_t offset = 0;
    while (offset < text.size()) {
        ZyanStatus status = ZydisDecoderDecodeFull(
            &decoder,
            text.data() + offset,
            text.size() - offset,
            &instr,
            operands
        );

        if (!ZYAN_SUCCESS(status)) {
            offset++; // resync in case of invalid byte
            continue;
        }

        freq[ZydisMnemonicGetString(instr.mnemonic)]++;
        offset += instr.length;
    }

    std::ofstream fout(out);
    if (!fout) {
        std::cerr << "Could not open output file\n";
        return 1;
    }

    for (const auto& [mnemonic, count] : freq) {
        fout << mnemonic << " " << count << "\n";
    }

    std::cout << "Wrote instruction frequency to " << out << "\n";
    return 0;
}