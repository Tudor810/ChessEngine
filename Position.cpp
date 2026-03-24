#include "Position.h"
#include <sstream>
#include <map>

Position::Position(std::string fenString) {
	
	std::stringstream ss(fenString);
	std::string t;

	for (int i = 0; i < 64; i++) {
		board[i] = -1;
	}
	
	for (int i = 0; i < 12; i++) {
		pieceBB[i] = 0;
		colorBB[0] = colorBB[1] = 0;
		occupiedBB = 0;
	}
	gameState = 0;
	zobristKey = 0;
	
	int cnt = 0;

	while (ss >> t) {
		if (cnt == 0) {
			int rank = 7, file = 0;

			for (char c : t) {
				if (c == '/') {
					rank--;
					file = 0;
				}
				else if (std::isdigit(c)) {
					file += (c - '0');
				}
				else {
					int square = rank * 8 + file;
					int pType = -1;

					switch (c) {
						case 'P': pType = 0;  break;
						case 'N': pType = 1;  break;
						case 'B': pType = 2;  break;
						case 'R': pType = 3;  break;
						case 'Q': pType = 4;  break;
						case 'K': pType = 5;  break;
						case 'p': pType = 6;  break;
						case 'n': pType = 7;  break;
						case 'b': pType = 8;  break;
						case 'r': pType = 9;  break;
						case 'q': pType = 10; break;
						case 'k': pType = 11; break;
						default: break; 
					}
					if (pType != -1) {
						board[square] = pType % 6;
						pieceBB[pType] |= C64(1) << square;
						colorBB[pType / 6] |= C64(1) << square;
						file++;
					}
					
				}			
			}
		} else if (cnt == 1) {
			setSideToMove(t[0] == 'w' ? WHITE : BLACK);
		}
		else if (cnt == 2) {
			int castleRights = 0;
			if (t.find('K') != std::string::npos) castleRights |= 1;
			if (t.find('Q') != std::string::npos) castleRights |= 2;
			if (t.find('k') != std::string::npos) castleRights |= 4;
			if (t.find('q') != std::string::npos) castleRights |= 8;
			setCastlingRights(castleRights);
		}
		else if (cnt == 3) {
			if (t == "-") setEnPassantSq(64);
			else {
				int file = t[0] - 'a';
				int rank = t[1] - '1'; // Fixed: '1' instead of '0'
				setEnPassantSq(rank * 8 + file);
			}
		}
		else if (cnt == 4) {
			setHalfMove(std::stoi(t));
		}
		else if (cnt == 5) {
			setFullMove(std::stoi(t));
		}
		cnt += 1;
	} 

	occupiedBB = colorBB[0] | colorBB[1];
}

bool Position::isSquareAttacked(int sq, Color enemyColor) const {

	Color us = (Color)(enemyColor ^ 1);

	// Attacked by Pawns 
	if (PawnAttacks[us][sq] & getPieces(enemyColor, PAWN)) return true;	

	// Attacked by Knights
	if (KnightAttacks[sq] & getPieces(enemyColor, KNIGHT)) return true;

	// Attacked by Bishops or Queen
	if(getBishopAttacks(sq, getOccupancy()) & (getPieces(enemyColor, BISHOP) | getPieces(enemyColor, QUEEN))) return true;

	// Attacked by Rook or Queen

	if (getRookAttacks(sq, getOccupancy()) & (getPieces(enemyColor, ROOK) | getPieces(enemyColor, QUEEN))) return true;

	// Attacked by King
	if (KingAttacks[sq] & getPieces(enemyColor, KING)) return true;

	// Not attacked 
	return false;
}

void Position::makeMove(Move move) {


	history[gamePly].gameState = gameState;
	history[gamePly].zobristKey = zobristKey;
	gamePly++;

	Color us = getSideToMove();
	Color them = (Color)(us ^ 1);
	int idxUs = us * 6;
	int idxThem = them * 6;

	int from = MoveUtils::getFrom(move);
	int to = MoveUtils::getTo(move);

	U64 fromBB = C64(1) << from;
	U64 toBB = C64(1) << to;
	U64 fromToBB = fromBB ^ toBB;

	int movePiece = MoveUtils::getMovePiece(move);
	int capturePiece = MoveUtils::getCapturePiece(move);
	int flags = MoveUtils::getFlags(move);

	pieceBB[movePiece + idxUs] ^= fromToBB;
	colorBB[us] ^= fromToBB;	
	board[to] = board[from];
	board[from] = -1;

	if (capturePiece != EMPTY) {
		pieceBB[capturePiece + idxThem] ^= toBB;
		colorBB[them] ^= toBB;
	}
	
	setEnPassantSq(64);
	int castlingRights = getCastlingRights();

	switch (flags) {

		case MoveFlag::EnPassant: {
			int epSquare = (us == WHITE ? to - 8 : to + 8);
			U64 epBit = C64(1) << epSquare;
			pieceBB[PAWN + idxThem] ^= epBit;
			colorBB[them] ^= epBit;
			board[epSquare] = -1;
			break;
		}
		case MoveFlag::KingCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << H1) ^ (C64(1) << F1))   // H1 to F1
				: ((C64(1) << H8) ^ (C64(1) << F8)); // H8 to F8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? F1 : F8] = ROOK;
			board[(us == WHITE) ? H1 : H8] = -1;
			break;
		}

		case MoveFlag::QueenCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << A1) ^ (C64(1) << D1))   // A1 to D1
				: ((C64(1) << A8) ^ (C64(1) << D8)); // A8 to D8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? D1 : D8] = ROOK;
			board[(us == WHITE) ? A1 : A8] = -1;
			break;
		}
		case MoveFlag::PromoteCaptureQueen:
		case MoveFlag::PromoteQueen: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[QUEEN + idxUs] ^= toBB;
			board[to] = QUEEN;
			break;
		}
		
		case MoveFlag::PromoteCaptureKnight:
		case MoveFlag::PromoteKnight: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[KNIGHT + idxUs] ^= toBB;
			board[to] = KNIGHT;
			break;
		}

		case MoveFlag::PromoteCaptureBishop:
		case MoveFlag::PromoteBishop: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[BISHOP + idxUs] ^= toBB;
			board[to] = BISHOP;
			break;
		}

		case MoveFlag::PromoteCaptureRook:
		case MoveFlag::PromoteRook: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[ROOK + idxUs] ^= toBB;
			board[to] = ROOK;
			break;
		}

		case MoveFlag::DoublePawnPush: {
			setEnPassantSq(us == WHITE ? to - 8 : to + 8);
			break;
		}
	}
	
	occupiedBB = colorBB[0] | colorBB[1];

	setCastlingRights(castlingRights & CastlingMasks[to] & CastlingMasks[from]);

	setSideToMove(them);
}

void Position::unmakeMove(Move move) {

	gamePly--;
	gameState = history[gamePly].gameState;
	zobristKey = history[gamePly].zobristKey;

	Color us = getSideToMove();
	Color them = (Color)(us ^ 1);
	int idxUs = us * 6;
	int idxThem = them * 6;

	int from = MoveUtils::getFrom(move);
	int to = MoveUtils::getTo(move);
	int movePiece = MoveUtils::getMovePiece(move);
	int capturePiece = MoveUtils::getCapturePiece(move);
	int flags = MoveUtils::getFlags(move);

	U64 fromBB = C64(1) << from;
	U64 toBB = C64(1) << to;
	U64 fromToBB = fromBB ^ toBB;

	pieceBB[movePiece + idxUs] ^= fromToBB;
	colorBB[us] ^= fromToBB;

	board[from] = movePiece;
	board[to] = (capturePiece == EMPTY) ? -1 : capturePiece;

	if (capturePiece != EMPTY) {
		pieceBB[capturePiece + idxThem] ^= toBB;
		colorBB[them] ^= toBB;
	}


	switch (flags) {
		case MoveFlag::EnPassant: {
			int epSquare = (us == WHITE ? to - 8 : to + 8);
			U64 epBit = C64(1) << epSquare;
			pieceBB[PAWN + idxThem] ^= epBit;
			colorBB[them] ^= epBit;
			board[epSquare] = PAWN;	
			break;
		}
		case MoveFlag::KingCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << H1) ^ (C64(1) << F1))   // H1 to F1
				: ((C64(1) << H8) ^ (C64(1) << F8)); // H8 to F8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? H1 : H8] = ROOK;
			board[(us == WHITE) ? F1 : F8] = -1;
			break;
		}

		case MoveFlag::QueenCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << A1) ^ (C64(1) << D1))   // A1 to D1
				: ((C64(1) << A8) ^ (C64(1) << D8)); // A8 to D8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? A1 : A8] = ROOK;
			board[(us == WHITE) ? D1 : D8] = -1;
			break;
		}
		case MoveFlag::PromoteCaptureQueen:
		case MoveFlag::PromoteQueen: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[QUEEN + idxUs] ^= toBB;
			break;
		}

		case MoveFlag::PromoteCaptureKnight:
		case MoveFlag::PromoteKnight: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[KNIGHT + idxUs] ^= toBB;
			break;
		}

		case MoveFlag::PromoteCaptureBishop:
		case MoveFlag::PromoteBishop: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[BISHOP + idxUs] ^= toBB;
			break;
		}

		case MoveFlag::PromoteCaptureRook:
		case MoveFlag::PromoteRook: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[ROOK + idxUs] ^= toBB;
			break;
		}
	}

	occupiedBB = colorBB[0] | colorBB[1];

	setSideToMove(us);

}