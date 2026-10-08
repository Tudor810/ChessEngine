// Perft: counts every leaf of the legal move tree to a fixed depth and
// compares it with the published numbers for standard test positions.
// A mismatch means a bug in move generation or make/unmake.
#include "board/Position.h"
#include "movegen/MoveGenerator.h"
#include "core/Zobrist.h"

#include <chrono>
#include <cstdio>
#include <memory>

static unsigned long long perft(Position& pos, int depth) {
    if (depth == 0) return 1;

    MoveList list;
    MoveGenerator gen;
    gen.genAllMoves(pos, list);

    unsigned long long nodes = 0;
    for (int i = 0; i < list.size(); i++) {
        pos.makeMove(list[i]);
        if (!pos.isInCheck(ENEMY))  // skip moves that leave our own king in check
            nodes += perft(pos, depth - 1);
        pos.unmakeMove(list[i]);
    }
    return nodes;
}

int main() {
    zobrist.init();
    initMoveData();

    struct Test { const char* name; const char* fen; int depth; unsigned long long expected; };
    const Test tests[] = {
        {"Start position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609ULL},
        {"Kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603ULL},
        {"Position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5, 674624ULL},
        {"Position 4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333ULL},
        {"Position 5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487ULL},
    };

    int failures = 0;
    for (const Test& t : tests) {
        auto pos = std::make_unique<Position>(t.fen);
        auto start = std::chrono::steady_clock::now();
        unsigned long long nodes = perft(*pos, t.depth);
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        bool ok = nodes == t.expected;
        if (!ok) failures++;
        std::printf("%-15s depth %d: %10llu (expected %10llu)  %s  %6.0f ms\n",
                    t.name, t.depth, nodes, t.expected, ok ? "OK" : "FAIL", ms);
    }
    return failures == 0 ? 0 : 1;
}
