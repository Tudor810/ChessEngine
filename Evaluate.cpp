#include "Evaluate.h"
#include "BitboardUtils.h"

namespace Evaluate {
	const int PawnValue   = 100;
	const int KnightValue = 320;
	const int BishopValue = 330;
	const int RookValue   = 500;
	const int QueenValue  = 900;
    const int KingValue   = 20000;

    const int pawnPST[64] = {
      0,  0,  0,  0,  0,  0,  0,  0,
     50, 50, 50, 50, 50, 50, 50, 50,
     10, 10, 20, 30, 30, 20, 10, 10,
      5,  5, 10, 25, 25, 10,  5,  5,
      0,  0,  0, 20, 20,  0,  0,  0,
      5, -5,-10,  0,  0,-10, -5,  5,
      5, 10, 10,-20,-20, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
    };
    const int knightPST[64] = {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50
    };
    const int bishopPST[64] = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };
    const int rookPST[64] = {
          0,  0,  0,  0,  0,  0,  0,  0,
          5, 10, 10, 10, 10, 10, 10,  5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
         -5,  0,  0,  0,  0,  0,  0, -5,
          0,  0,  0,  5,  5,  0,  0,  0
    };
    const int queenPST[64] = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };
    const int kingPST[64] = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };


    int evaluate(const Position& pos) {
        int score = 0;

        for (int square = 0; square < 64; square++) {
            int piece = pos.getPieceAt(square);

            if (piece == -1) continue;

            int pieceColor = MailBoxUtils::getColor(piece);
            int pieceType = MailBoxUtils::getPiece(piece);

            // If it's Black, flip the square index to read the table from Black's perspective
            int tableIndex = (pieceColor == WHITE) ? (square ^ 56) : (square);

            int materialValue = 0;
            int pstBonus = 0;

            switch (pieceType) {
            case PAWN:   materialValue = PawnValue; pstBonus = pawnPST[tableIndex]; break;
            case KNIGHT: materialValue = KnightValue; pstBonus = knightPST[tableIndex]; break;
            case BISHOP: materialValue = BishopValue; pstBonus = bishopPST[tableIndex]; break;
            case ROOK:   materialValue = RookValue; pstBonus = rookPST[tableIndex]; break;
            case QUEEN:  materialValue = QueenValue; pstBonus = queenPST[tableIndex]; break;
            case KING:   materialValue = KingValue; pstBonus = kingPST[tableIndex]; break;
            }

            // Add to White's score, subtract from Black's score
            if (pieceColor == WHITE) {
                score += (materialValue + pstBonus);
            }
            else {
                score -= (materialValue + pstBonus);
            }
        }

        // Flip perspective for Negamax
        if (pos.getSideToMove() == BLACK) {
            return -score;
        }

        return score;
    }
}