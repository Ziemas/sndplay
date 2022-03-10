#include "musplay.h"
#include "midi.h"
#include <cassert>
#include <cstdio>
#include <fmt/format.h>
#include <iterator>
#include <memory>

std::vector<soundbank> gBanks;

int loadSongs()
{
    return 0;
}

void soundbank::load_seq()
{
    auto midi = (MIDI*)seqBuf.get();

    fmt::print("seq fourcc {:.4}\n", (char*)midi);

    if (midi->mmid.fourcc == FOURCC('M', 'M', 'I', 'D')) {
        fmt::print("multitrack\n");
        for (int i = 0; i < midi->mmid.nTrack; i++) {
            fmt::print("offset {}: {}\n", i, midi->mmid.MID[i]);
            MID* m = (MID*)(seqBuf.get() + midi->mmid.MID[i]);
            fmt::print("fourcc {:.4}\n", (char*)m);
        }
    }

    if (midi->mid.fourcc == FOURCC('M', 'I', 'D', ' ')) {
        fmt::print("single track mid\n");

        midi_player player((u8*)midi + midi->mid.offset);
        player.play();
    }
}

void soundbank::load(sbv2Data* bank)
{
    data = *bank;

    assert(data.fourcc == FOURCC('S', 'B', 'v', '2'));

    auto instD = (instrumentData*)((u8*)bank + bank->instrumentOffset);

    for (int i = 0; i < bank->instrumentCount; i++) {
        Instrument instrument;
        instrument.data = instD[i];

        auto region = (regionData*)((u8*)bank + instD[i].oRegion);

        for (int j = 0; j < instD[i].nRegion; j++)
            instrument.regions.push_back(region[j]);

        instruments.push_back(instrument);
    }

    auto songD = (songData*)((u8*)bank + bank->songOffset);

    for (int i = 0; i < bank->songCount; i++) {
        songs.push_back(songD[i]);
    }
}

static int load_bank(char* filename)
{
    std::fstream file;
    file.open(filename, std::fstream::binary | std::fstream::in);

    file.seekg(0, std::fstream::beg);

    if (!file.is_open()) {
        fmt::print("Failed to open file\n");
        return -1;
    }

    sbHeaderData header;
    file.read((char*)&header, sizeof(sbHeaderData));

    // idk if this is what is signified by this at all...
    if (header.Type != FileType::SBv2 && header.Type != FileType::SBlk) {
        fmt::print("{}: Unsupported type: {}\n", filename, (int)header.Type);
        return -1;
    }

    u32 size = header.sbSize;
    if (header.version >= 3) {
        // Don't know why yet
        size += 4;
    }

    auto* soundBank = (sbv2Data*)malloc(size);
    if (soundBank == nullptr) {
        fmt::print("malloc failed\n");
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
    file.read((char*)soundBank, header.sbSize);

    soundbank sb;
    sb.load(soundBank);

    if (header.sampleSize == 0) {
        fmt::print("{}: bank size {}\n", filename, header.sampleSize);
        fmt::print("unhandled!\n");
        return 0;
    }

    u8* sampleBuf = (u8*)malloc(header.sampleSize);
    if (soundBank == nullptr) {
        fmt::print("sample malloc failed\n");
        return -1;
    }

    file.seekg(header.sampleOffset, std::fstream::beg);
    sb.sampleBuf = std::make_unique<u8[]>(header.sampleSize);
    file.read((char*)sb.sampleBuf.get(), header.sampleSize);

    seqData sqData;
    file.seekg(header.seqOffset, std::fstream::beg);
    file.read((char*)&sqData, sizeof(seqData));

    sb.seqBuf = std::make_unique<u8[]>(sqData.size);
    file.read((char*)sb.seqBuf.get(), sqData.size);

    sb.load_seq();

    gBanks.push_back(std::move(sb));

    free(sampleBuf);
    free(soundBank);

    file.close();

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc > 1)
        load_bank(argv[1]);

    for (auto& b : gBanks) {
        //fmt::print("Bank {:.4}\n", (char*)&b.data.name);
        //fmt::print("Read {} instruments, {} song(s)\n", b.instruments.size(), b.songs.size());

        // fmt::print("instruments:\n");
        // for (auto &i : b.instruments) {
        //     fmt::print("regions: {}\n", i.data.nRegion);
        //     //for (auto &r : i.regions) {
        //     //    fmt::print("keymap {:x}\n", r.keymap);
        //     //    fmt::print("sample  {:x}\n", r.oSample);
        //     //    fmt::print("marker {:x}\n", r.marker2);
        //     //}
        //     fmt::print("vol?: {:x}\n", i.data.volume);
        //     fmt::print("unk: {}\n", i.data.something);
        //     fmt::print("regionidx: {}\n", i.data.oRegion);
        //     fmt::print("----:\n");
        // }

        //fmt::print("Songs:\n");
        //for (auto& s : b.songs) {
        //    fmt::print("type {}\n", s.type);
        //    fmt::print("bank {:.4}\n", (char*)&s.bankName);
        //    fmt::print("midi {:.4}\n", (char*)&s.midiName);
        //    fmt::print("unk {:.4}\n", (char*)&s.unkName);
        //    fmt::print("{}\n", s.midiIdx);
        //}
    }

    return 0;
}
