#include "player.h"

int main(int argc, char* argv[])
{
    snd_player player;
    u32 bankid = 0;

    if (argc > 1) {
        bankid = player.load_bank(argv[1]);
        player.play_sound(bankid, 0);
    }


    // for (auto& b : gBanks) {
    //     fmt::print("Bank {:.4}\n", (char*)&b.data.BankID);

    //    fmt::print("Songs:\n");
    //    for (auto& s : b.sounds) {
    //        fmt::print("type {}\n", s.Type);
    //        fmt::print("bank {:.4}\n", (char*)&s.Bank);
    //        fmt::print("midi {:.4}\n", (char*)&s.MIDIID);
    //        fmt::print("unk {:.4}\n", (char*)&s.OrigBank);
    //        fmt::print("{}\n", s.Index);
    //    }
    //}

    return 0;
}
