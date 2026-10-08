# ChessEngine

A chess engine written from scratch in C++. It uses bitboards and talks the **UCI protocol**, so you can plug it into any chess GUI (Cute Chess, Arena, En Croissant, …) and play against it or run engine matches.

## Features

**Board and move generation**
- Bitboard board representation (one 64-bit mask per piece type and colour)
- Precomputed knight and king attack tables
- **Magic bitboards** for sliding pieces (rooks, bishops, queens)
- Full rules: castling, en passant, promotions, legal-move filtering
- **Zobrist hashing** with incremental updates in make/unmake

**Search**
- Negamax with **alpha-beta pruning**
- **Iterative deepening**, reporting `info depth … score cp … nodes … nps …` after each depth
- **Quiescence search** on captures (ordered by MVV-LVA), to avoid the horizon effect
- **Transposition table** (about 1M entries) storing exact, lower- and upper-bound scores and the best move
- **Null-move pruning** (adaptive reduction `R = 2 + depth / 4`, disabled in check, in pawn-only endgames and near mate scores)
- Move ordering: the transposition-table move first, then captures by **MVV-LVA** (most valuable victim, least valuable attacker)
- Mate scores adjusted by distance, so the engine prefers the fastest mate
- Draw detection: threefold repetition, the 50-move rule and insufficient material

**Evaluation**
- **Tapered evaluation**: separate middlegame and endgame piece values and piece-square tables (PeSTO values), blended by game phase
- **Mop-up** term in winning endgames: pushes the losing king to the edge and brings the winning king closer, so the engine actually delivers mate

**Time management**
- Each move gets roughly `remaining time / 30 + increment / 2`; the search stops when the budget runs out and plays the best move from the last completed depth

## Correctness: perft

Move generation is checked with **perft**: count every leaf of the legal move tree to a fixed depth and compare against the published numbers. The engine matches all five standard test positions:

| Position | Depth | Nodes | Result |
|---|---|---|---|
| Start position | 5 | 4,865,609 | ✅ |
| Kiwipete | 4 | 4,085,603 | ✅ |
| Position 3 | 5 | 674,624 | ✅ |
| Position 4 | 4 | 422,333 | ✅ |
| Position 5 | 4 | 2,103,487 | ✅ |

Run it yourself with `ctest` (or the `perft` executable) after building. These positions are designed to hit the edge cases (castling through check, en passant pins, promotions with capture), so passing all five is strong evidence the move generator is correct.

## Building

The project uses **CMake** (3.20+) and a C++20 compiler (MSVC, GCC or Clang).

**Visual Studio 2022 / 2026:** choose **File → Open → Folder** and open the repository folder. Visual Studio detects `CMakeLists.txt`, configures the project, and lets you pick `ChessEngine.exe` or `perft.exe` as the startup item. Use the **x64-Release** configuration; Debug builds are much slower.

**Command line (Windows, Linux, macOS):**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release      # runs the perft test
```

The engine executable is `build/ChessEngine` (or `build/Release/ChessEngine.exe` with the Visual Studio generator).

## Usage

The engine speaks UCI over standard input and output. The easiest way to use it is to add the executable as an engine in a GUI such as [Cute Chess](https://cutechess.com/) or [En Croissant](https://encroissant.org/).

You can also talk to it directly in a terminal:

```
uci
uciok
isready
readyok
position startpos moves e2e4 e7e5
go depth 8
info depth 1 score cp ... nodes ... nps ...
...
bestmove g1f3
```

Supported commands:

| Command | What it does |
|---|---|
| `uci` | Identifies as a UCI engine (`uciok`) |
| `isready` | Replies `readyok` |
| `position startpos [moves …]` | Sets up the start position, optionally followed by moves in UCI notation (`e2e4`, `e7e8q`) |
| `position fen <fen> [moves …]` | Sets up any position from a FEN string |
| `go depth N` | Searches to a fixed depth |
| `go wtime … btime … winc … binc …` | Searches with a clock, using the time-management rule above |
| `quit` | Exits |

## Code structure

```
ChessEngine/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              UCI command loop
│   ├── core/                 Types.h, Move.h (types and move encoding)
│   │                         BitboardUtils.*  (bit tricks, attack tables, magic bitboards)
│   │                         Zobrist.*        (hash keys)
│   ├── board/                Position.*       (board state, FEN, make/unmake, check and draw detection)
│   ├── movegen/              MoveGenerator.*  (pseudo-legal move generation)
│   ├── search/               Search.*, TT.h   (iterative deepening, alpha-beta, quiescence, null move, TT, time management)
│   └── eval/                 Evaluate.*       (tapered PeSTO evaluation and mop-up)
└── tests/
    └── perft.cpp             move-generation test against standard positions
```

Everything except `main.cpp` is built as a static library (`engine`), so the engine and the tests share the same code. Includes are relative to `src/`, for example `#include "board/Position.h"`.

## Roadmap

- Killer moves and history heuristic for better quiet-move ordering
- Late move reductions and principal variation search
- Strength testing with Cute Chess against other engines to get an Elo estimate
- `go movetime`, `go infinite` and `stop`
