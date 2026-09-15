#pragma once

typedef unsigned long long  U64;
#define C64(constantU64) constantU64##ULL
constexpr auto ENEMY = 1;;
constexpr auto ALLY  = 0;

enum Color { WHITE = 0, BLACK = 1 };
enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, EMPTY };
enum Square {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

struct StateInfo {
	int gameState = 0;      
	/* bits 0-3: Castling byte 0 - WK, byte 1 - WQ, byte 2 - BK, byte 3 - BQ
	   bits 4-10: EP Square
	   bits 11-17: Halfmove
	   bit 18: Side to move
	   bit 19 - 31: FullMove
	*/
	U64 zobristKey = 0;     // The 64-bit hash of the position
};

typedef int Move;
/*
bits 0-5: from
bits 6-11: to
bits 12-15: flags
bits 16-18: Move Piece 000 - Pawn, 001 - Knight, 010 - Bishop, 011 - Rook, 100 - Queen, 101 - King, 110 - None
bits 19-21: Capture Piece 000 - Pawn, 001 - Knight, 010 - Bishop, 011 - Rook, 100 - Queen, 101 - King, 110 - None
*/
