#pragma once

#include "Types.h"

struct ZobristKeys {
    U64 pieces[12][64];
    U64 side;
    U64 castling[16];
    U64 ep[65];

    void init();
};

extern ZobristKeys zobrist;
