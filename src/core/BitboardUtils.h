#pragma once

#include <bit>
#include "core/Types.h"

const int epMask = 0x7F;
const int castleMask = 0xF;
const int halfMoveMask = 0x7F;
const int fullMoveMask = 0x1FF;


const U64 notAFile = C64(0xfefefefefefefefe); // Every bit except A-column
const U64 notABFile = C64(0xfcfcfcfcfcfcfcfc); // Every bit except A, B - column
const U64 notHFile = C64(0x7f7f7f7f7f7f7f7f); // Every bit except H-column
const U64 notGHFile = C64(0x3f3f3f3f3f3f3f3f); // Every bit except G, H - column
const U64 innerBoard = C64(0x007e7e7e7e7e7e00); // All bits are 1 except for the top/bottom ranks and left/right files
const U64 rank4 = C64(0x00000000FF000000);
const U64 rank5 = C64(0x000000FF00000000);
const U64 rank8 = C64(0xFF00000000000000);
const U64 rank1 = C64(0x00000000000000FF);

extern U64 RookMasks[64];
extern U64 BishopMasks[64];

extern U64 PawnAttacks[2][64];
extern U64 KnightAttacks[64];
extern U64 KingAttacks[64];
extern U64 RookAttacks[64][4096];
extern U64 BishopAttacks[64][512];

const int CastlingMasks[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,  // A1 is 13 (~2), E1 is 12 (~3), H1 is 14 (~1)
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	 7, 15, 15, 15,  3, 15, 15, 11   // A8 is 7 (~8), E8 is 3 (~12), H8 is 11 (~4)
};

const U64 RMagic[64] = {
    C64(0xa8002c000108020), C64(0x6c00049b0002001), C64(0x100200010090040), C64(0x2480041000800801),
    C64(0x280028004000800), C64(0x900410008040022), C64(0x280020001001080), C64(0x2880002041000080),
    C64(0xa000800080400034), C64(0x4808020004000), C64(0x2290802004801000), C64(0x411000d00100020),
    C64(0x402800800040080), C64(0xb000401004208), C64(0x2409000100040200), C64(0x1002100004082),
    C64(0x22878001e24000), C64(0x1090810021004010), C64(0x801030040200012), C64(0x500808008001000),
    C64(0xa08018014000880), C64(0x8000808004000200), C64(0x201008080010200), C64(0x801020000441091),
    C64(0x800080204005), C64(0x1040200040100048), C64(0x120200402082), C64(0xd14880480100080),
    C64(0x12040280080080), C64(0x100040080020080), C64(0x9020010080800200), C64(0x813241200148449),
    C64(0x491604001800080), C64(0x100401000402001), C64(0x4820010021001040), C64(0x400402202000812),
    C64(0x209009005000802), C64(0x810800601800400), C64(0x4301083214000150), C64(0x204026458e001401),
    C64(0x40204000808000), C64(0x8001008040010020), C64(0x8410820820420010), C64(0x1003001000090020),
    C64(0x804040008008080), C64(0x12000810020004), C64(0x1000100200040208), C64(0x430000a044020001),
    C64(0x280009023410300), C64(0xe0100040002240), C64(0x200100401700), C64(0x2244100408008080),
    C64(0x8000400801980), C64(0x2000810040200), C64(0x8010100228810400), C64(0x2000009044210200),
    C64(0x4080008040102101), C64(0x40002080411d01), C64(0x2005524060000901), C64(0x502001008400422),
    C64(0x489a000810200402), C64(0x1004400080a13), C64(0x4000011008020084), C64(0x26002114058042)
};

const U64 BMagic[64] = {
    C64(0x89a1121896040240), C64(0x2004844802002010), C64(0x2068080051921000), C64(0x62880a0220200808),
    C64(0x4042004000000), C64(0x100822020200011), C64(0xc00444222012000a), C64(0x28808801216001),
    C64(0x400492088408100), C64(0x201c401040c0084), C64(0x840800910a0010), C64(0x82080240060),
    C64(0x2000840504006000), C64(0x30010c4108405004), C64(0x1008005410080802), C64(0x8144042209100900),
    C64(0x208081020014400), C64(0x4800201208ca00), C64(0xf18140408012008), C64(0x1004002802102001),
    C64(0x841000820080811), C64(0x40200200a42008), C64(0x800054042000), C64(0x88010400410c9000),
    C64(0x520040470104290), C64(0x1004040051500081), C64(0x2002081833080021), C64(0x400c00c010142),
    C64(0x941408200c002000), C64(0x658810000806011), C64(0x188071040440a00), C64(0x4800404002011c00),
    C64(0x104442040404200), C64(0x511080202091021), C64(0x4022401120400), C64(0x80c0040400080120),
    C64(0x8040010040820802), C64(0x480810700020090), C64(0x102008e00040242), C64(0x809005202050100),
    C64(0x8002024220104080), C64(0x431008804142000), C64(0x19001802081400), C64(0x200014208040080),
    C64(0x3308082008200100), C64(0x41010500040c020), C64(0x4012020c04210308), C64(0x208220a202004080),
    C64(0x111040120082000), C64(0x6803040141280a00), C64(0x2101004202410000), C64(0x8200000041108022),
    C64(0x21082088000), C64(0x2410204010040), C64(0x40100400809000), C64(0x822088220820214),
    C64(0x40808090012004), C64(0x910224040218c9), C64(0x402814422015008), C64(0x90014004842410),
    C64(0x1000042304105), C64(0x10008830412a00), C64(0x2520081090008908), C64(0x40102000a0a60140)
};

const int RBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};
const int BBits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

void initMoveData();
void initMasks();

inline U64 nortOne(U64 b) { return b << 8; }
inline U64 soutOne(U64 b) { return b >> 8; }
inline U64 eastOne(U64 b) { return (b << 1) & notAFile; }
inline U64 westOne(U64 b) { return (b >> 1) & notHFile; }

inline U64 noEaOne(U64 b) { return (b << 9) & notAFile; }
inline U64 noWeOne(U64 b) { return (b << 7) & notHFile; }
inline U64 soEaOne(U64 b) { return (b >> 7) & notAFile; }
inline U64 soWeOne(U64 b) { return (b >> 9) & notHFile; }

inline U64 wSinglePushTargets(U64 wpawns, U64 empty) { return nortOne(wpawns) & empty; }
inline U64 bSinglePushTargets(U64 bpawns, U64 empty) { return soutOne(bpawns) & empty; }

inline int bitScanForward(U64 bb) { return std::countr_zero(bb); }
inline int countSetBits(U64 bb) { return std::popcount(bb); }

inline U64 rankMask(int sq) { return C64(0xFF) << (sq & 56); }
inline U64 fileMask(int sq) { return C64(0x0101010101010101) << (sq & 7); }

inline U64 diagonalMask(int sq) {
	const U64 maindia = C64(0x8040201008040201);
	int diag = (sq & 7) - (sq >> 3);
	return diag >= 0 ? maindia >> diag * 8 : maindia << -diag * 8;
}

namespace MailBoxUtils {
    inline int getColor(int piece) { return piece / 6; }
    inline int getPiece(int piece) { return piece % 6; }
}


inline U64 antiDiagMask(int sq) {
	const U64 maindia = C64(0x0102040810204080);
	int diag = 7 - (sq & 7) - (sq >> 3);
	return diag >= 0 ? maindia >> diag * 8 : maindia << -diag * 8;
}

inline U64 rookMaskEx(int sq) {
	U64 rank = rankMask(sq);
	U64 file = fileMask(sq);

	U64 internalFiles = C64(0x7E7E7E7E7E7E7E7E); 
	U64 internalRanks = C64(0x00FFFFFFFFFFFF00);

	U64 clippedRank = rank & internalFiles;
	U64 clippedFile = file & internalRanks;

	return (clippedRank | clippedFile) & ~(C64(1) << sq);
}

inline U64 bishopMaskEx(int sq) { return (diagonalMask(sq) | antiDiagMask(sq)) & innerBoard & ~(C64(1) << sq); }
inline U64 getRookAttacks(int sq, U64 occupancy) { return RookAttacks[sq][((occupancy & RookMasks[sq]) * RMagic[sq]) >> (64 - RBits[sq])]; }
inline U64 getBishopAttacks(int sq, U64 occupancy) { return BishopAttacks[sq][((occupancy & BishopMasks[sq]) * BMagic[sq]) >> (64 - BBits[sq])]; }


class Position;

namespace CastlingRules {
	bool canCastleWK(const Position& pos);
	bool canCastleWQ(const Position& pos);
	bool canCastleBK(const Position& pos);
	bool canCastleBQ(const Position& pos);
}