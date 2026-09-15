#include "BitboardUtils.h"
#include "Position.h"
#include <random>

U64 KnightAttacks[64];
U64 KingAttacks[64];
U64 RookMasks[64];
U64 BishopMasks[64];
U64 RookAttacks[64][4096];
U64 BishopAttacks[64][512];
U64 PawnAttacks[2][64];

void initMasks() {
	for (int sq = 0; sq < 64; sq++) {
		RookMasks[sq] = rookMaskEx(sq);
		BishopMasks[sq] = bishopMaskEx(sq);
	}
}

U64 computeKnightAttacks(int sq) {
	U64 knight = C64(1) << sq;
	U64 attacks = 0;

	attacks |= (knight << 17) & notAFile; // NoNoEa 
	attacks |= (knight << 15) & notHFile; // NoNoWe
	attacks |= (knight << 6) & notGHFile; // NoWeWe
	attacks |= (knight << 10) & notABFile; // NoEaEa
	attacks |= (knight >> 15) & notAFile; // SoSoEa 
	attacks |= (knight >> 17) & notHFile; // SoSoWe
	attacks |= (knight >> 6) & notABFile; // SoEaEa
	attacks |= (knight >> 10) & notGHFile; // SoWeWe

	return attacks;
}

U64 computeKingAttacks(int sq) {
	U64 king = C64(1) << sq;
	U64 attacks = 0;

	attacks |= nortOne(king) | soutOne(king) | eastOne(king) | westOne(king) | noEaOne(king) | noWeOne(king) | soEaOne(king) | soWeOne(king);

	return attacks;
}

U64 computeRookAttacks(int sq, U64 blockers) {
	U64 attacks = 0;
	int r = sq / 8; // Rank
	int f = sq % 8; // File 

	// North 
	for (int i = r + 1; i < 8; i++) {
		U64 bit = C64(1) << (i * 8 + f);

		attacks |= bit;
		if (blockers & bit) break;
	}

	// South
	for (int i = r - 1; i >= 0; i--) {
		U64 bit = C64(1) << (i * 8 + f);
		attacks |= bit;
		if (blockers & bit) break;
	}

	// East 
	for (int i = f + 1; i < 8; i++) {
		U64 bit = C64(1) << (r * 8 + i);
		attacks |= bit;
		if (blockers & bit) break;
	}

	// West
	for (int i = f - 1; i >= 0; i--) {
		U64 bit = C64(1) << (r * 8 + i);
		attacks |= bit;
		if (blockers & bit) break;
	}
	return attacks;
}

U64 computeBishopAttacks(int sq, U64 blockers) {
	U64 attacks = 0;
	int r = sq / 8; // rank
	int f = sq % 8; // file 

	// North - East

	for (int i = r + 1, j = f + 1; i < 8 && j < 8; i++, j++) {
		U64 bit = C64(1) << (i * 8 + j);
		attacks |= bit;
		if (blockers & bit) break;
	}

	// South - West

	for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) {
		U64 bit = C64(1) << (i * 8 + j);
		attacks |= bit;
		if (blockers & bit) break;
	}

	// North - West 

	for (int i = r + 1, j = f - 1; i < 8 && j >= 0; i++, j--) {
		U64 bit = C64(1) << (i * 8 + j);
		attacks |= bit;
		if (blockers & bit) break;
	}

	// South - East
	for (int i = r - 1, j = f + 1; i >= 0 && j < 8; i--, j++) {
		U64 bit = C64(1) << (i * 8 + j);
		attacks |= bit;
		if (blockers & bit) break;
	}

	return attacks;
}

void computePawnAttacks(int sq) {
	U64 pawn = C64(1) << sq;

	// White 
	PawnAttacks[WHITE][sq] |= noEaOne(pawn);
	PawnAttacks[WHITE][sq] |= noWeOne(pawn);

	// Black
	PawnAttacks[BLACK][sq] |= soEaOne(pawn);
	PawnAttacks[BLACK][sq] |= soWeOne(pawn);
}

U64 setOccupancy(int index, int bits, U64 mask) {

	U64 occupancy = 0;

	for (int i = 0; i < bits; i++) {
		int sq = bitScanForward(mask);
		mask &= mask - 1;
		if (index & (1 << i)) occupancy |= C64(1) << sq;
	}

	return occupancy;
}

void initRookTables(int sq) {
	U64 rMask = RookMasks[sq];
	int bits = RBits[sq];
	int numPermutations = 1 << bits;
	for (int i = 0; i < numPermutations; i++) { // Generates all possible blockers 
		U64 blockers = setOccupancy(i, bits, rMask); 
		int magicIndex = (int)((blockers * RMagic[sq]) >> (64 - bits)); // Calculate a unique index for each position

		U64 attacks = computeRookAttacks(sq, blockers);
		 // Sets the moves

		if (RookAttacks[sq][magicIndex] != 0 && RookAttacks[sq][magicIndex] != attacks) {
			printf("FATAL: Rook Destructive collision at Square %d, Index %d!\n", sq, magicIndex);
		}

		RookAttacks[sq][magicIndex] = attacks;
	}
}

void initBishopTables(int sq) {
	U64 bMask = BishopMasks[sq];
	int bits = BBits[sq];
	int numPermutations = 1 << bits;
	for (int i = 0; i < numPermutations; i++) { // Generates all possible blockers 
		U64 blockers = setOccupancy(i, bits, bMask);
		int magicIndex = (int)((blockers * BMagic[sq]) >> (64 - bits)); // Calculate a unique index for each position

		U64 attacks = computeBishopAttacks(sq, blockers);
		 // Sets the moves
		if (BishopAttacks[sq][magicIndex] != 0 && BishopAttacks[sq][magicIndex] != attacks) {
			printf("FATAL: Destructive collision at Square %d, Index %d!\n", sq, magicIndex);
		}
		BishopAttacks[sq][magicIndex] = attacks;
	}
}

void initMoveData() {

	initMasks();
	for (int sq = 0; sq < 64; sq++) {
		KnightAttacks[sq] = computeKnightAttacks(sq);
		KingAttacks[sq] = computeKingAttacks(sq);
		initRookTables(sq);
		initBishopTables(sq);
		computePawnAttacks(sq);
	}
}


bool CastlingRules::canCastleWK(const Position& pos) {
	if (pos.getOccupancy() & C64(0x60)) return false;

	return !pos.isSquareAttacked(E1, BLACK) && 
               !pos.isSquareAttacked(F1, BLACK) && 
               !pos.isSquareAttacked(G1, BLACK);
}

bool CastlingRules::canCastleWQ(const Position& pos) {
	if (pos.getOccupancy() & C64(0x0E)) return false;

	return !pos.isSquareAttacked(E1, BLACK) &&
		!pos.isSquareAttacked(D1, BLACK) &&
		!pos.isSquareAttacked(C1, BLACK);
}

bool CastlingRules::canCastleBK(const Position& pos) {
	if (pos.getOccupancy() & C64(0x6000000000000000)) return false;

	return !pos.isSquareAttacked(E8, WHITE) &&
		!pos.isSquareAttacked(F8, WHITE) &&
		!pos.isSquareAttacked(G8, WHITE);
}

bool CastlingRules::canCastleBQ(const Position& pos) {
	if (pos.getOccupancy() & C64(0x0E00000000000000)) return false;

	return !pos.isSquareAttacked(E8, WHITE) &&
		!pos.isSquareAttacked(D8, WHITE) &&
		!pos.isSquareAttacked(C8, WHITE);
}
