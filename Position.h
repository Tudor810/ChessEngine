#pragma once

#include "Types.h"
#include "BitboardUtils.h"
#include "Move.h"
#include <string>


class Position
{
	int board[64];
	U64	pieceBB[12];
	U64 colorBB[2];
	U64 occupiedBB;
	
	//int mailbox[64];
	
	StateInfo history[1024]{};
	int gamePly = 0;

	int gameState;
	U64 zobristKey;
	/* bits 0-3: Castling byte 0 - WK, byte 1 - WQ, byte 2 - BK, byte 3 - BQ
	   bits 4-10: EP Square
	   bits 11-17: Halfmove
	   bit 18: Side to move
	   bit 19 - 31: FullMove
	*/

	public:

		Position(std::string fenString);

		inline U64 getPieces(Color c, PieceType p) const { return pieceBB[p + (6 * c)];}
		inline U64 getOccupancy() const { return occupiedBB; }
		inline Color getSideToMove() const { return (Color)((gameState >> 18) & 1); }
		
		inline U64 getColor(Color c) const { return colorBB[c]; }
		inline int getEnPassantSq() const { 
			int sq = (gameState >> 4) & epMask; 
			return (sq == 64) ? -1 : sq;
		}
		inline int getCastlingRights() const { return gameState & castleMask;  }
		inline int getHalfMove() const { return (gameState >> 11) & halfMoveMask; }
		inline int getFullMove() const { return (gameState >> 19) & fullMoveMask; }

		inline int getPieceAt(int index) const {
			return board[index];
		}

		inline int getPieceOnSquare(int index) const {
			int p = board[index];
			if (p == -1) return 6; // 6 is EMPTY
			return p % 6;
		}

		inline void setSideToMove(Color c) {
			gameState &= ~(1 << 18);
			gameState |= (c << 18);  
		}
		inline void setEnPassantSq(int enSq) {
			gameState &= ~(epMask << 4);
			gameState |= (enSq << 4);   
		}
		inline void setCastlingRights(int castlingRights) {
			gameState &= ~castleMask;   
			gameState |= castlingRights; 
		}
		inline void setHalfMove(int halfMove) {
			gameState &= ~(halfMoveMask << 11);
			gameState |= (halfMove << 11);
		}
		inline void setFullMove(int fullMove) {
			gameState &= ~(fullMoveMask << 19);
			gameState |= (fullMove << 19);
		}

		inline int getKingSquare(int type) const { return bitScanForward(getPieces((Color)(getSideToMove() ^ type), KING)); }

		bool isSquareAttacked(int sq, Color enemyColor) const;
		inline bool isInCheck(int type) const { return isSquareAttacked(getKingSquare(type), (Color)(getSideToMove() ^ (1 ^ type))); }
		
		// Added for debugging
		//void printChessBoard();

		U64 generateZobristKey() const;

		bool isDraw() const;

		void makeMove(Move move);			
		void unmakeMove(Move move);

};

 