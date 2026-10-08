#pragma once
//Move.h 
#include <string>

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

    constexpr Move MOVE_NULL = 0;
    inline Move encode(int from, int to, int flags, int movePiece, int capPiece) {
        return from | (to << 6) | (flags << 12) | (movePiece << 16) | (capPiece << 19);
    }

    inline int getFrom(Move m) { return m & 0x3F; }
    inline int getTo(Move m) { return (m >> 6) & 0x3F; }
    inline int getFlags(Move m) { return (m >> 12) & 0xF; }
    inline int getMovePiece(Move move) { return (move >> 16) & 0x7; }
    inline int getCapturePiece(Move move) { return (move >> 19) & 0x7; }

    inline std::string printSquare(int sq) {
        std::string s = "";
        s += (char)('a' + (sq % 8));
        s += (char)('1' + (sq / 8));
        return s;
    }

    inline std::string printMove(Move m) {
        if (m == 0) return "0000";
        std::string moveStr = printSquare(getFrom(m)) + printSquare(getTo(m));

        int flags = getFlags(m);
        // UCI requires appending a letter for promotions
        if (flags == MoveFlag::PromoteQueen || flags == MoveFlag::PromoteCaptureQueen) moveStr += 'q';
        else if (flags == MoveFlag::PromoteRook || flags == MoveFlag::PromoteCaptureRook) moveStr += 'r';
        else if (flags == MoveFlag::PromoteBishop || flags == MoveFlag::PromoteCaptureBishop) moveStr += 'b';
        else if (flags == MoveFlag::PromoteKnight || flags == MoveFlag::PromoteCaptureKnight) moveStr += 'n';

        return moveStr;
    }

};


struct MoveList {
    Move moves[256]{};
    int scores[256]{};
    int count = 0;

    inline void push(int from, int to, int flag, int movePiece, int capPiece) {
        moves[count++] = MoveUtils::encode(from, to, flag, movePiece, capPiece);
    }

    inline void push(Move move) {
        moves[count++] = move;
    }
    inline Move& operator[](int index) { return moves[index]; }
    int size() const { return count; }
};
