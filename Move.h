#pragma once
//Move.h 

namespace MoveFlag {

    const int Quiet = 0;
    const int DoublePawnPush = 1;
    const int KingCastle = 2;
    const int QueenCastle = 3;
    const int Capture = 4;
    const int EnPassant = 5;
    const int PromoteKnight = 8;
    const int PromoteBishop = 9;
    const int PromoteRook = 10;
    const int PromoteQueen = 11;
    const int PromoteCaptureKnight = 12;
    const int PromoteCaptureBishop = 13;
    const int PromoteCaptureRook = 14;
    const int PromoteCaptureQueen = 15;
};

namespace MoveUtils {
    inline Move encode(int from, int to, int flags, int movePiece, int capPiece) {
        return from | (to << 6) | (flags << 12) | (movePiece << 16) | (capPiece << 19);
    }

    inline int getFrom(Move m) { return m & 0x3F; }
    inline int getTo(Move m) { return (m >> 6) & 0x3F; }
    inline int getFlags(Move m) { return (m >> 12) & 0xF; }
    inline int getMovePiece(Move move) { return (move >> 16) & 0x7; }
    inline int getCapturePiece(Move move) { return (move >> 19) & 0x7; }

};


struct MoveList {
    Move moves[256]{};
    int count = 0;

    inline void push(int from, int to, int flag, int movePiece, int capPiece) {
        moves[count++] = MoveUtils::encode(from, to, flag, movePiece, capPiece);
    }

    inline Move operator[](int index) const { return moves[index]; }
    int size() const { return count; }
};
