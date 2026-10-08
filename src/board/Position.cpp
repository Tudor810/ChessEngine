#include "board/Position.h"
#include "core/Zobrist.h"

#include <sstream>
//#include <map>
//#include <iostream> // Added for debugging 
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
						board[square] = pType;
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
	zobristKey = generateZobristKey();

}

bool Position::hasNonPawnMaterial(Color sideToMove) const {
	return (getPieces(sideToMove, KNIGHT) |
		getPieces(sideToMove, BISHOP) |
		getPieces(sideToMove, ROOK) |
		getPieces(sideToMove, QUEEN)) != C64(0);
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

bool Position::isDraw() const {


	// Half Move rule ( 50 moves without pawn move or capture) 

	if (getHalfMove() >= 100) {
		return true;
	}

	// Insufficient material rule 
	if (getPieces(WHITE, PAWN) | getPieces(BLACK, PAWN) |
		getPieces(WHITE, ROOK) | getPieces(BLACK, ROOK) |
		getPieces(WHITE, QUEEN) | getPieces(BLACK, QUEEN)) {
		// Do nothing, material is sufficient
	}
	else {
		// Only Kings, Knights, and Bishops remain.
		int whiteMinorCount = countSetBits(getPieces(WHITE, KNIGHT)) + countSetBits(getPieces(WHITE, BISHOP));
		int blackMinorCount = countSetBits(getPieces(BLACK, KNIGHT)) + countSetBits(getPieces(BLACK, BISHOP));

		if (std::abs(whiteMinorCount - blackMinorCount) <= 1) {
			return true;
		}
	}


	// Three fold repetition rule
	int lookback = getHalfMove() > pliesFromNull ? pliesFromNull : getHalfMove();
	int limit = gamePly - lookback;

	if (limit < 0) limit = 0;

	for (int i = gamePly - 2; i >= limit; i -= 2) {
		if (history[i].zobristKey == this->zobristKey) {
			return true;
		}
	}

	// Position not draw
	return false;
}

void Position::makeMove(Move move) {


	// Save the game state
	history[gamePly].gameState = gameState;
	history[gamePly].zobristKey = zobristKey;
	history[gamePly].pliesFromNull = pliesFromNull;
	gamePly++;
	pliesFromNull++;

	int oldEp = getEnPassantSq();
	zobristKey ^= zobrist.ep[(oldEp == -1) ? 64 : oldEp]; // Remove OLD EP
	zobristKey ^= zobrist.castling[getCastlingRights()]; // Remove OLD Castling Rights

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

	// Handling regular moves 

	pieceBB[movePiece + idxUs] ^= fromToBB;
	colorBB[us] ^= fromToBB;	
	board[to] = board[from];
	board[from] = -1;

	zobristKey ^= zobrist.pieces[movePiece + idxUs][from]; // Remove piece from
	zobristKey ^= zobrist.pieces[movePiece + idxUs][to]; // Add piece to


	// Handling regular captures
	if (capturePiece != EMPTY) {
		pieceBB[capturePiece + idxThem] ^= toBB;
		colorBB[them] ^= toBB;
		zobristKey ^= zobrist.pieces[capturePiece + idxThem][to]; // Remove captured piece
	}
	
	setEnPassantSq(64);
	int castlingRights = getCastlingRights();


	// Half Move and Full Move logic 

	if (movePiece == PAWN || capturePiece != EMPTY) {
		setHalfMove(0);
	}
	else {
		setHalfMove(getHalfMove() + 1);
	}

	if (us == BLACK) {
		setFullMove(getFullMove() + 1);
	}
	// Special Moves Logic 
	switch (flags) {

		case MoveFlag::EnPassant: {
			int epSquare = (us == WHITE ? to - 8 : to + 8);
			U64 epBit = C64(1) << epSquare;
			pieceBB[PAWN + idxThem] ^= epBit;
			colorBB[them] ^= epBit;
			board[epSquare] = -1;
			zobristKey ^= zobrist.pieces[PAWN + idxThem][epSquare]; // Remove en passant captured pawn
			break;
		}
		case MoveFlag::KingCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << H1) ^ (C64(1) << F1))   // H1 to F1
				: ((C64(1) << H8) ^ (C64(1) << F8)); // H8 to F8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? F1 : F8] = ROOK + idxUs;
			board[(us == WHITE) ? H1 : H8] = -1;
			zobristKey ^= zobrist.pieces[ROOK + idxUs][(us == WHITE) ? H1 : H8]; // Remove rook from H
			zobristKey ^= zobrist.pieces[ROOK + idxUs][(us == WHITE) ? F1 : F8]; // Add rook to F
			break;
		}

		case MoveFlag::QueenCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << A1) ^ (C64(1) << D1))   // A1 to D1
				: ((C64(1) << A8) ^ (C64(1) << D8)); // A8 to D8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? D1 : D8] = ROOK + idxUs;
			board[(us == WHITE) ? A1 : A8] = -1;
			zobristKey ^= zobrist.pieces[ROOK + idxUs][(us == WHITE) ? A1 : A8]; // Remove rook from A
			zobristKey ^= zobrist.pieces[ROOK + idxUs][(us == WHITE) ? D1 : D8]; // Add rook to D
			break;
		}
		case MoveFlag::PromoteCaptureQueen:
		case MoveFlag::PromoteQueen: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[QUEEN + idxUs] ^= toBB;
			board[to] = QUEEN + idxUs;
			zobristKey ^= zobrist.pieces[PAWN + idxUs][to]; // Remove Pawn
			zobristKey ^= zobrist.pieces[QUEEN + idxUs][to]; // Add Queen
			break;
		}
		
		case MoveFlag::PromoteCaptureKnight:
		case MoveFlag::PromoteKnight: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[KNIGHT + idxUs] ^= toBB;
			board[to] = KNIGHT + idxUs;
			zobristKey ^= zobrist.pieces[PAWN + idxUs][to]; // Remove Pawn
			zobristKey ^= zobrist.pieces[KNIGHT + idxUs][to]; // Add Knight
			break;
		}

		case MoveFlag::PromoteCaptureBishop:
		case MoveFlag::PromoteBishop: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[BISHOP + idxUs] ^= toBB;
			board[to] = BISHOP + idxUs;
			zobristKey ^= zobrist.pieces[PAWN + idxUs][to]; // Remove Pawn
			zobristKey ^= zobrist.pieces[BISHOP + idxUs][to]; // Add Bishop
			break;
		}

		case MoveFlag::PromoteCaptureRook:
		case MoveFlag::PromoteRook: {
			pieceBB[PAWN + idxUs] ^= toBB;
			pieceBB[ROOK + idxUs] ^= toBB;
			board[to] = ROOK + idxUs;
			zobristKey ^= zobrist.pieces[PAWN + idxUs][to]; // Remove Pawn
			zobristKey ^= zobrist.pieces[ROOK + idxUs][to]; // Add Rook
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

	int newEp = getEnPassantSq();

	zobristKey ^= zobrist.ep[(newEp == -1) ? 64 : newEp]; // add new ep
	zobristKey ^= zobrist.castling[getCastlingRights()]; // add new castling 
	zobristKey ^= zobrist.side; // add new side to move 
}

void Position::unmakeMove(Move move) {

	gamePly--;
	gameState = history[gamePly].gameState;
	zobristKey = history[gamePly].zobristKey;
	pliesFromNull = history[gamePly].pliesFromNull;

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

	board[from] = movePiece + idxUs;
	board[to] = (capturePiece == EMPTY) ? -1 : capturePiece + idxThem;

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
			board[epSquare] = PAWN + idxThem;	
			break;
		}
		case MoveFlag::KingCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << H1) ^ (C64(1) << F1))   // H1 to F1
				: ((C64(1) << H8) ^ (C64(1) << F8)); // H8 to F8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;
			board[(us == WHITE) ? H1 : H8] = ROOK + idxUs;
			board[(us == WHITE) ? F1 : F8] = -1;
			break;
		}

		case MoveFlag::QueenCastle: {
			U64 rookFromTo = (us == WHITE) ? ((C64(1) << A1) ^ (C64(1) << D1))   // A1 to D1
				: ((C64(1) << A8) ^ (C64(1) << D8)); // A8 to D8
			pieceBB[ROOK + idxUs] ^= rookFromTo;
			colorBB[us] ^= rookFromTo;	
			board[(us == WHITE) ? A1 : A8] = ROOK + idxUs;
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

U64 Position::generateZobristKey() const {
	U64 key = 0;
	for (int sq = 0; sq < 64; sq++) {
		int piece = board[sq];
		if (piece != -1) key ^= zobrist.pieces[piece][sq];
	}

	if (getSideToMove() == BLACK) key ^= zobrist.side;

	key ^= zobrist.castling[getCastlingRights()];
	int epSq = getEnPassantSq();

	key ^= zobrist.ep[(epSq == -1) ? 64 : epSq];

	return key;
}

void Position::make_null_move() {
	history[gamePly].gameState = gameState;
	history[gamePly].zobristKey = zobristKey;
	history[gamePly].pliesFromNull = pliesFromNull;
	gamePly++;
	pliesFromNull = 0;

	Color us = getSideToMove();
	Color them = (Color)(us ^ 1);

	int oldEp = getEnPassantSq();
	zobristKey ^= zobrist.ep[(oldEp == -1) ? 64 : oldEp]; // Remove OLD EP
	zobristKey ^= zobrist.ep[64];

	setHalfMove(getHalfMove() + 1);
	if (us == BLACK) {
		setFullMove(getFullMove() + 1);
	}

	zobristKey ^= zobrist.side;
	setSideToMove(them);
	setEnPassantSq(64);
}

void Position::unmake_null_move() {
	gamePly--;
	gameState = history[gamePly].gameState;
	zobristKey = history[gamePly].zobristKey;
	pliesFromNull = history[gamePly].pliesFromNull;
}	

// Added for debugging 

//void Position::printChessBoard() {
//	// Mapping arrays based on IDs:
//	// 0-5:   p, n, b, r, q, k (White)
//	// 6-11:  P, N, B, R, Q, K (Black)
//	char pieces[] = { 'p', 'n', 'b', 'r', 'q', 'k',
//					 'P', 'N', 'B', 'R', 'Q', 'K' };
//
//	std::cout << "  +------------------------+\n";
//
//	// Loop from row 7 down to 0 to print from White's perspective (Rank 8 to 1)
//	for (int i = 7; i >= 0; --i) {
//		std::cout << (i + 1) << " |";
//		for (int j = 0; j < 8; ++j) {
//			int pieceId = MailBoxUtils::getPiece(this -> board[i * 8 + j]) + 6 * MailBoxUtils::getColor(this->board[i * 8 + j]);
//
//			if (pieceId == -1) {
//				std::cout << "  .";
//			}
//			else if (pieceId >= 0 && pieceId <= 11) {
//				std::cout << "  " << pieces[pieceId];
//			}
//			else {
//				std::cout << "  ?"; // Fallback for invalid IDs
//			}
//		}
//		std::cout << " |\n";
//	}
//
//	std::cout << "  +------------------------+\n";
//	std::cout << "     a  b  c  d  e  f  g  h\n";
//}