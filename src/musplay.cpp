#include "musplay.h"
#include <cstdio>
#include <fstream>
#include <memory>

std::fstream file;

int loadSongs() { return 0; }

int loadBank(char *filename) {

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

    auto *soundBank = (SoundBank *)malloc(size);
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

    free(sampleBuf);
    free(soundBank);

    file.close();

    return 0;
}

int main(int argc, char *argv[]) {
    return 0;
}
