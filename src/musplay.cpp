#include "musplay.h"
#include <cstdio>
#include <iterator>
#include <memory>

std::vector<SoundBank> gBanks;

int loadSongs() { return 0; }

void SoundBank::Load(sbv2Struct *bank) {
    sbParams = *bank;

    auto instr = (Instrument *)((u8 *)bank + bank->instrumentOffset);

    for (int i = 0; i < bank->instrumentCount; i++) {
        auto region = (Region *)((u8 *)bank + instr->oRegion);

        std::vector<Region> rvec;
        for (int j = 0; i < instr->nRegion; i++)
            rvec.push_back(region[j]);

        regions.emplace(i, rvec);
        instruments.push_back(instr[i]);
    }
}

int loadBank(char *filename) {
    std::fstream file;
    file.open(filename, std::fstream::binary | std::fstream::in);
    file.seekg(0, std::fstream::beg);

    if (!file.is_open()) {
        printf("Failed to open file\n");
        return -1;
    }

    sbHeader header;
    file.read((char *)&header, sizeof(sbHeader));

    // idk if this is what is signified by this at all...
    if (header.Type != FileType::SBv2 && header.Type != FileType::SBlk) {
        printf("%s: Unsupported type: %d\n", filename, (int)header.Type);
        return -1;
    }

    u32 size = header.sbSize;
    if (header.version >= 3) {
        // Don't know why yet
        size += 4;
    }

    auto *soundBank = (sbv2Struct *)malloc(size);
    if (soundBank == nullptr) {
        printf("malloc failed\n");
        return -1;
    }

    // The following seems pointless because we read into an offset pointer
    // and then never use the base ptr for anything again.
    /*
    u32 *dst = bankBuf;
     if (header.subVer == 3)
        dst = bankBuf + 1;
    file.read((char *)dst, size);
    */

    file.seekg(header.bankOffset, std::fstream::beg);
    file.read((char *)soundBank, header.sbSize);

    SoundBank sb;
    sb.Load(soundBank);
    gBanks.push_back(sb);

    if (header.sampleSize == 0) {
        printf("%s: bank size %d\n", filename, header.sampleSize);
        printf("unhandled!\n");
        return 0;
    }

    u8 *sampleBuf = (u8 *)malloc(header.sampleSize);
    if (soundBank == nullptr) {
        printf("sample malloc failed\n");
        return -1;
    }

    file.seekg(header.sampleOffset, std::fstream::beg);
    file.read((char *)sampleBuf, header.sampleSize);
    for (u32 i = 0; i < header.sampleSize; i++)
        sb.samplebuf.push_back(sampleBuf[i]);

    free(sampleBuf);
    free(soundBank);

    file.close();

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1)
        loadBank(argv[1]);

    for (auto b : gBanks) {
        printf("fourcc: %.*s\n", 4, (char *)&b.sbParams.fourcc);
        printf("bank: %.*s\n", 4, (char *)&b.sbParams.name);
        //printf("instruments:\n");
        //for (auto i : b.instruments) {
        //    printf("regions: %x\n", i.nRegion);
        //    printf("vol?: %x\n", i.volume);
        //    printf("unk: %x\n", i.something);
        //    printf("regionidx: %x\n", i.oRegion);
        //    printf("----:\n");
        //}
    }

    return 0;
}
