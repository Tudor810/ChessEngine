#include "MoveGenerator.h"

void MoveGenerator::genPawnMoves(const Position& pos, MoveList& list) {
	Color us = pos.getSideToMove();
	U64 empty = ~pos.getOccupancy();

	if (us == WHITE) {
		U64 wPawns = pos.getPieces(WHITE, PAWN);

		// 1. Handle Single Pushes 

		U64 singlePushTargets = wSinglePushTargets(wPawns, empty);
		U64 pushPromos = singlePushTargets & rank8;
		U64 pushQuiets = singlePushTargets & ~rank8;

		while (pushPromos) {
			int to = bitScanForward(pushPromos);
			int from = to - 8; // For a white single push, 'from' is 1 rank south

			list.push(from, to, MoveFlag::PromoteQueen, PAWN, EMPTY);
			list.push(from, to, MoveFlag::PromoteRook, PAWN, EMPTY);
			list.push(from, to, MoveFlag::PromoteBishop, PAWN, EMPTY);
			list.push(from, to, MoveFlag::PromoteKnight, PAWN, EMPTY);
			pushPromos &= pushPromos  - 1; // Clear bit 
		}

		while (pushQuiets) {
			int to = bitScanForward(pushQuiets);
			int from = to - 8; // For a white single push, 'from' is 1 rank south
			
			list.push(from, to, MoveFlag::Quiet, PAWN, EMPTY);
			pushQuiets &= pushQuiets - 1; // Clear bit 
		}

		//2. Handle Double Pushes 

		U64 doublePushTargets = wSinglePushTargets(singlePushTargets, empty) & rank4; // Make sure the pawns came from rank 2

		while (doublePushTargets) {
			int to = bitScanForward(doublePushTargets);
			int from = to - 16; // For a white double push, 'from' is 2 rank south
			list.push(from, to, MoveFlag::DoublePawnPush, PAWN, EMPTY);
			doublePushTargets &= doublePushTargets - 1;
		}

		//3. Handle Captures
		int epSq = pos.getEnPassantSq();
		U64 enPassantMask = (epSq == -1) ? 0 : (C64(1) << epSq);
		U64 captureMask = pos.getColor(BLACK) | enPassantMask;
		U64 attackEastTargets = noEaOne(wPawns) & captureMask;
		U64 eastPromos = attackEastTargets & rank8;
		U64 eastNormal = attackEastTargets & ~rank8;

		while (eastPromos) {
			int to = bitScanForward(eastPromos);
			int from = to - 9; // East capture for white
			
			list.push(from, to, MoveFlag::PromoteCaptureQueen, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureRook, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureBishop, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureKnight, PAWN, pos.getPieceOnSquare(to));
		
			eastPromos &= eastPromos - 1;
		}
		while (eastNormal) {
			int to = bitScanForward(eastNormal);
			int from = to - 9;
			list.push(from, to, (to == epSq) ? MoveFlag::EnPassant : MoveFlag::Capture, PAWN, (to == epSq) ? EMPTY : pos.getPieceOnSquare(to));
			eastNormal &= eastNormal - 1;
		}

		U64 attackWestTargets = noWeOne(wPawns) & captureMask;
		U64 westPromos = attackWestTargets & rank8;
		U64 westNormal = attackWestTargets & ~rank8;

		while (westPromos) {
			int to = bitScanForward(westPromos);
			int from = to - 7;
			list.push(from, to, MoveFlag::PromoteCaptureQueen, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureRook, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureBishop, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureKnight, PAWN, pos.getPieceOnSquare(to));
			westPromos &= westPromos - 1;
		}

		while (westNormal) {
			int to = bitScanForward(westNormal);
			int from = to - 7;
			list.push(from, to, (to == epSq) ? MoveFlag::EnPassant : MoveFlag::Capture, PAWN, (to == epSq) ? EMPTY : pos.getPieceOnSquare(to));
			westNormal &= westNormal - 1;
		}
	}
	else { // Same Logic for Black
		U64 bPawns = pos.getPieces(BLACK, PAWN);

		// 1. Handle Single Pushes 

		U64 singlePushTargets = bSinglePushTargets(bPawns, empty);
		U64 pushPromos = singlePushTargets & rank1;
		U64 pushQuiets = singlePushTargets & ~rank1;

		while (pushPromos) {
			int to = bitScanForward(pushPromos);
			int from = to + 8; // For a black single push, 'from' is 1 rank north
			list.push(from, to, MoveFlag::PromoteQueen, PAWN, EMPTY);
			list.push(from, to, MoveFlag::PromoteRook, PAWN, EMPTY);
			list.push(from, to, MoveFlag::PromoteBishop, PAWN, EMPTY);
			list.push(from, to, MoveFlag::PromoteKnight, PAWN, EMPTY);
			
			pushPromos &= pushPromos - 1; // Clear bit 
		}

		while (pushQuiets) {
			int to = bitScanForward(pushQuiets);
			list.push(to + 8, to, MoveFlag::Quiet, PAWN, EMPTY);
			pushQuiets &= pushQuiets - 1;
		}

		//2. Handle Double Pushes 

		U64 doublePushTargets = bSinglePushTargets(singlePushTargets, empty) & rank5; // Make sure the pawns came from rank 7

		while (doublePushTargets) {
			int to = bitScanForward(doublePushTargets);
			int from = to + 16; // For a black double push, 'from' is 2 rank north
			list.push(from, to, MoveFlag::DoublePawnPush, PAWN, EMPTY);
			doublePushTargets &= doublePushTargets - 1;
		}

		//3. Handle Captures
		int epSq = pos.getEnPassantSq();
		U64 enPassantMask = (epSq == -1) ? 0 : (C64(1) << epSq);
		U64 captureMask = pos.getColor(WHITE) | enPassantMask;
		U64 attackEastTargets = soEaOne(bPawns) & captureMask;
		U64 eastPromos = attackEastTargets & rank1;
		U64 eastNormal = attackEastTargets & ~rank1;

		while (eastPromos) {
			int to = bitScanForward(eastPromos);
			int from = to + 7; // East capture for black

			list.push(from, to, MoveFlag::PromoteCaptureQueen, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureRook, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureBishop, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureKnight, PAWN, pos.getPieceOnSquare(to));
			eastPromos &= eastPromos - 1;
		}

		while (eastNormal) {
			int to = bitScanForward(eastNormal);
			int from = to + 7;

			list.push(from, to, (to == epSq) ? MoveFlag::EnPassant : MoveFlag::Capture, PAWN, (to == epSq) ? EMPTY : pos.getPieceOnSquare(to));
			eastNormal &= eastNormal - 1;
		}

		U64 attackWestTargets = soWeOne(bPawns) & captureMask;
		U64 westPromos = attackWestTargets & rank1;
		U64 westNormal = attackWestTargets & ~rank1;

		while (westPromos) {
			int to = bitScanForward(westPromos);
			int from = to + 9; // West capture for black

			list.push(from, to, MoveFlag::PromoteCaptureQueen, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureRook, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureBishop, PAWN, pos.getPieceOnSquare(to));
			list.push(from, to, MoveFlag::PromoteCaptureKnight, PAWN, pos.getPieceOnSquare(to));
				
			westPromos &= westPromos - 1;
		}

		while (westNormal) {
			int to = bitScanForward(westNormal);
			int from = to + 9;
			list.push(from, to, (to == epSq) ? MoveFlag::EnPassant : MoveFlag::Capture, PAWN, (to == epSq) ? EMPTY : pos.getPieceOnSquare(to));
			westNormal &= westNormal - 1;
		}
	}

}

void MoveGenerator::genKnightMoves(const Position& pos, MoveList& list) {
	Color us = pos.getSideToMove();
	Color them = (Color)(us ^ 1);
	U64 knights = pos.getPieces(us, KNIGHT);
	U64 allied = pos.getColor(us);
	U64 enemies = pos.getColor(them);

	while (knights) {
		int from = bitScanForward(knights);

		U64 attacks = KnightAttacks[from] & ~allied;

		U64 captures = attacks & enemies;
		U64 quiets = attacks & ~enemies;

		while (captures) {
			int to = bitScanForward(captures);
			list.push(from, to, MoveFlag::Capture, KNIGHT, pos.getPieceOnSquare(to));
			captures &= captures - 1;
		}
		while (quiets) {
			int to = bitScanForward(quiets);
			list.push(from, to, MoveFlag::Quiet, KNIGHT, EMPTY);
			quiets &= quiets - 1;
		}
		knights &= knights - 1;
	}
}

void MoveGenerator::genKingMoves(const Position& pos, MoveList& list) {
	Color us = pos.getSideToMove();
	Color them = (Color)(us ^ 1);
	U64 king = pos.getPieces(us, KING);
	U64 allied = pos.getColor(us);
	U64 enemies = pos.getColor(them);

	int from = bitScanForward(king);

	U64 attacks = KingAttacks[from] & ~allied;
	U64 captures = attacks & enemies;
	U64 quiets = attacks & ~enemies;

	while (captures) {
		int to = bitScanForward(captures);
		list.push(from, to, MoveFlag::Capture, KING, pos.getPieceOnSquare(to));
		captures &= captures - 1;
	}
	while (quiets) {
		int to = bitScanForward(quiets);
		list.push(from, to, MoveFlag::Quiet, KING, EMPTY);
		quiets &= quiets - 1;
	}

	// Castling 
	int rights = pos.getCastlingRights();

	if (us == WHITE) {
		if (rights & 0x3) {
			if ((rights & 1) && CastlingRules::canCastleWK(pos)) list.push(E1, G1, MoveFlag::KingCastle, KING, EMPTY);
			if ((rights & 2) && CastlingRules::canCastleWQ(pos)) list.push(E1, C1, MoveFlag::QueenCastle, KING, EMPTY);
		}
	}
	else {
		if (rights & 0xC) {
			if ((rights & 4) && CastlingRules::canCastleBK(pos)) list.push(E8, G8, MoveFlag::KingCastle, KING, EMPTY);
			if ((rights & 8) && CastlingRules::canCastleBQ(pos)) list.push(E8, C8, MoveFlag::QueenCastle, KING, EMPTY);
		}
	}
}

void MoveGenerator::genBishopMoves(const Position& pos, MoveList& list) {\
	
	Color us = pos.getSideToMove();
	Color them = (Color)(us ^ 1);
	U64 bishops = pos.getPieces(us, BISHOP);
	U64 occupancy = pos.getOccupancy();
	U64 allied = pos.getColor(us);
	U64 enemies = pos.getColor(them);

	while (bishops) {
		int from = bitScanForward(bishops);

		U64 attacks = getBishopAttacks(from, occupancy) & ~allied;

		U64 captures = attacks & enemies;
		U64 quiets = attacks & ~enemies;

		while (captures) {
			int to = bitScanForward(captures);
			list.push(from, to, MoveFlag::Capture, BISHOP, pos.getPieceOnSquare(to));
			captures &= captures - 1;
		}
		while (quiets) {
			int to = bitScanForward(quiets);
			list.push(from, to, MoveFlag::Quiet, BISHOP, EMPTY);
			quiets &= quiets - 1;
		}
		
		bishops &= bishops - 1;
	}
}

void MoveGenerator::genRookMoves(const Position& pos, MoveList& list) {
	Color us = pos.getSideToMove();
	Color them = (Color)(us ^ 1);
	U64 rooks = pos.getPieces(us, ROOK);
	U64 occupancy = pos.getOccupancy();
	U64 allied = pos.getColor(us);
	U64 enemies = pos.getColor(them);

	while (rooks) {
		int from = bitScanForward(rooks);

		U64 attacks = getRookAttacks(from, occupancy) & ~allied;

		U64 captures = attacks & enemies;
		U64 quiets = attacks & ~enemies;

		while (captures) {
			int to = bitScanForward(captures);
			list.push(from, to, MoveFlag::Capture, ROOK, pos.getPieceOnSquare(to));
			captures &= captures - 1;
		}
		while (quiets) {
			int to = bitScanForward(quiets);
			list.push(from, to, MoveFlag::Quiet, ROOK, EMPTY);
			quiets &= quiets - 1;
		}

		rooks &= rooks - 1;
	}
}

void MoveGenerator::genQueenMoves(const Position& pos, MoveList& list) {
	Color us = pos.getSideToMove();
	Color them = (Color)(us ^ 1);
	U64 queens = pos.getPieces(us, QUEEN);
	U64 occupancy = pos.getOccupancy();
	U64 allied = pos.getColor(us);
	U64 enemies = pos.getColor(them);

	while (queens) {
		int from = bitScanForward(queens);

		U64 rookAttacks = getRookAttacks(from, occupancy);
		U64 bishopAttacks = getBishopAttacks(from, occupancy);

		U64 attacks = (rookAttacks | bishopAttacks) & ~allied;

		U64 captures = attacks & enemies;
		U64 quiets = attacks & ~enemies;

		while (captures) {
			int to = bitScanForward(captures);
			list.push(from, to, MoveFlag::Capture, QUEEN, pos.getPieceOnSquare(to));
			captures &= captures - 1;
		}
		while (quiets) {
			int to = bitScanForward(quiets);
			list.push(from, to, MoveFlag::Quiet, QUEEN, EMPTY);
			quiets &= quiets - 1;
		}

		queens &= queens - 1;
	}
}

void MoveGenerator::genAllMoves(const Position& pos, MoveList& list) {
	genPawnMoves(pos, list);
	genKnightMoves(pos, list);
	genBishopMoves(pos, list);
	genRookMoves(pos, list);
	genQueenMoves(pos, list);
	genKingMoves(pos, list);
}