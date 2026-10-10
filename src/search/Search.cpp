#include "search/Search.h"
#include "eval/Evaluate.h"
#include "search/TT.h"

#include <cstring>
//#include <iostream> // Added for debugging


TTEntry TT[TTSize];

const int mvvLva[6][6] = {
	{ 15, 25, 35, 45, 55, 0 }, // Pawn attacker
	{ 14, 24, 34, 44, 54, 0 }, // Knight attacker
	{ 13, 23, 33, 43, 53, 0 }, // Bishop attacker
	{ 12, 22, 32, 42, 52, 0 }, // Rook attacker
	{ 11, 21, 31, 41, 51, 0 }, // Queen attacker
	{ 10, 20, 30, 40, 50, 0 }  // King attacker
};

namespace {
	static constexpr int VALUE_MATED = -SHRT_MAX / 2; // -16383
	static constexpr int VALUE_MATE = SHRT_MAX / 2; //  16383
	static constexpr int VALUE_MATE_IN_MAX_PLY = VALUE_MATE - 256;
}

static inline int scoreToTT(int s, int ply) {
	return s > VALUE_MATE_IN_MAX_PLY ? s + ply : s < -VALUE_MATE_IN_MAX_PLY ? s - ply : s;
}
static inline int scoreFromTT(int s, int ply) {
	return s > VALUE_MATE_IN_MAX_PLY ? s - ply : s < -VALUE_MATE_IN_MAX_PLY ? s + ply : s;
}

void clearTT() {
	std::memset(TT, 0, sizeof(TT));   // needs <cstring>
}

Move Search::getBestMove(Position& pos, short depth, long long remTime, long long incTime) {


	// Initialization

	MoveList moves;
	moveGen.genAllMoves(pos, moves);
	nodes = 0;
	this->stopSearch = false;
	
	SearchStack* ss = &this->stack[0];
	ss->ply = 0;
	ss->currentMove = 1;

	// Filter out illegal moves at the root first & establish a fallback move
	MoveList legalRootMoves;
	for (int i = 0; i < moves.size(); i++) {
		pos.makeMove(moves[i]);
		if (!pos.isInCheck(ENEMY)) {
			legalRootMoves.push(moves[i]);
		}
		
		pos.unmakeMove(moves[i]);
	}

	// If checkmate or stalemate
	if (legalRootMoves.size() == 0) return 0;

	// Safety fallback: always have a legal move ready even if aborted at node 1
	Move finalBestMove = legalRootMoves[0];

	// Calculate time budget
	if (remTime != INT_MAX) {
		long long timeBudget = (remTime / 30) + (incTime / 2);
		this->deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeBudget);
	}
	else {
		// Infinite time / fixed depth mode
		this->deadline = std::chrono::steady_clock::time_point::max();
	}


	scoreMoves(legalRootMoves, 0);
	for (int i = 0; i < legalRootMoves.size() - 1; i++) {
		int bestIndex = i;
		for (int j = i + 1; j < legalRootMoves.size(); j++) {
			if (legalRootMoves.scores[j] > legalRootMoves.scores[bestIndex]) {
				bestIndex = j;
			}
		}
		std::swap(legalRootMoves[i], legalRootMoves[bestIndex]);
		std::swap(legalRootMoves.scores[i], legalRootMoves.scores[bestIndex]);
	}


	auto startTime = std::chrono::steady_clock::now();

	for (int d = 1; d <= depth; d++) {

		int alpha = -SHRT_MAX / 2;
		int beta = SHRT_MAX / 2;
		Move bestMoveThisDepth = 0;

		for (int i = 0; i < legalRootMoves.size(); i++) {

			pos.makeMove(legalRootMoves[i]);
			ss->currentMove = legalRootMoves[i];
			int score = -negamax(pos, ss + 1, d - 1, -beta, -alpha);
			pos.unmakeMove(legalRootMoves[i]);
	
			if (this -> stopSearch) {
				break;
			}
			if (score > alpha) {
				alpha = score;
				bestMoveThisDepth = legalRootMoves[i];
			}
		}

		if (!this->stopSearch) {
			auto currentTime = std::chrono::steady_clock::now();
			long long elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();

			// Prevent division by zero
			if (elapsed_ms == 0) elapsed_ms = 1;

			// Use 1000LL to prevent integer overflow on fast searches
			long long nps = (nodes * C64(1000)) / elapsed_ms;

			// Standard UCI format (Adding 'score cp' so your GUI shows the evaluation)
			printf("info depth %d score cp %d time %lld nodes %lld nps %lld\n",
				d, alpha, elapsed_ms, nodes, nps);
		}

		if (this -> stopSearch) {
			break;
		}

		if (bestMoveThisDepth != 0) {
			finalBestMove = bestMoveThisDepth;

			// Move ordering: bring the best move to index 0 for the next iteration
			for (int i = 0; i < legalRootMoves.size(); i++) {
				if (legalRootMoves[i] == finalBestMove) {
					std::swap(legalRootMoves[0], legalRootMoves[i]);
					break;
				}
			}
		}

	}
	
	return finalBestMove;
}

int Search::negamax(Position& pos, SearchStack* ss, int depth, int alpha, int beta) {
	

	ss->ply = (ss - 1)->ply + 1;
	if ((nodes & 2047) == 0) {
		if (std::chrono::steady_clock::now() >= this->deadline) {
			this->stopSearch = true;
			return 0;
		}
	}
	
	if (this->stopSearch) return 0;

	if (ss->ply > 0) {
		if (pos.isDraw()) {
			return 0;
		}
	}

	if (depth == 0) {
		return quiescence(pos, ss + 1, alpha, beta);
	}

	U64 zobristKey = pos.getZobristKey();
	TTEntry crtEntry = TT[zobristKey & (TTSize - 1)];
	Move firstMove = 0;

	uint32_t check = (uint32_t)(zobristKey >> 32);

	if (crtEntry.key == check) { // The position was already processed
		firstMove = crtEntry.bestMove;
		if (crtEntry.depth >= depth) {
			int ttScore = scoreFromTT(crtEntry.score, ss->ply); // ply adjustment
			if (crtEntry.flag == FLAG_EXACT)
				return ttScore;
			if (crtEntry.flag == FLAG_ALPHA && crtEntry.score <= alpha)
				return alpha;
			if (crtEntry.flag == FLAG_BETA && crtEntry.score >= beta)
				return beta;
		}
	}

	bool allowNull = (ss - 1)->currentMove != MoveUtils::MOVE_NULL;
	if (allowNull && depth >= 3 && 
		abs(beta) < VALUE_MATE_IN_MAX_PLY && 
		!pos.isInCheck(ALLY) && 
		pos.hasNonPawnMaterial(pos.getSideToMove())) {
		int r = 2 + depth / 4;

		ss->currentMove = MoveUtils::MOVE_NULL;
		pos.make_null_move();
		int score = -negamax(pos, ss + 1, depth - r - 1, -beta, -beta + 1);
		pos.unmake_null_move();

		if (this->stopSearch) return 0;

		if (score >= beta) {
			return beta;
		}
	}
	MoveList moves;
	moveGen.genAllMoves(pos, moves);

	scoreMoves(moves, firstMove);

	int legalMoves = 0;
	int originalAlpha = alpha;
	Move bestMoveInNode = 0;  

	for (int i = 0; i < moves.size(); i++) {


		int bestIndex = i;
		for (int j = i + 1; j < moves.size(); j++) {
			if (moves.scores[j] > moves.scores[bestIndex]) {
				bestIndex = j;
			}
		}
		std::swap(moves[i], moves[bestIndex]);
		std::swap(moves.scores[i], moves.scores[bestIndex]);

		pos.makeMove(moves[i]);

		if (pos.isInCheck(ENEMY)) {
			pos.unmakeMove(moves[i]);
			continue;
		}

		legalMoves++;
		nodes++;


		ss->currentMove = moves[i];
		int score = -negamax(pos, ss + 1, depth - 1, -beta, -alpha);
		pos.unmakeMove(moves[i]);

		if (this->stopSearch) {
			return 0;
		}
		if (score >= beta) {
			TT[zobristKey & (TTSize - 1)] = {check, (int16_t)scoreToTT(beta, ss->ply), (uint8_t)depth, FLAG_BETA, moves[i]};
			return beta;
		}
		if (score > alpha) {
			alpha = score;
			bestMoveInNode = moves[i];
		}
	}

	if (legalMoves == 0) {
		if (pos.isInCheck(ALLY)) { return VALUE_MATED + ss->ply; } // Checkmate
		
		return 0; // Stalemate
	}

	Move storeMove = bestMoveInNode;
	if (storeMove == 0 && crtEntry.key == check) storeMove = crtEntry.bestMove;
	
	TTFlag finalFlag = (alpha > originalAlpha) ? FLAG_EXACT : FLAG_ALPHA;
	TT[zobristKey & (TTSize - 1)] = {check, (int16_t)scoreToTT(beta, ss->ply), (uint8_t)depth, finalFlag, storeMove};


	return alpha;
}

int Search::quiescence(Position& pos, SearchStack* ss, int alpha, int beta) {

	ss->ply = (ss - 1)->ply + 1;


	if (ss->ply >= 255) return Evaluate::evaluate(pos);

	if ((nodes & 2047) == 0) {
		if (std::chrono::steady_clock::now() >= this->deadline) {
			this->stopSearch = true;
			return 0;
		}
	}

	if (this->stopSearch) return 0;
	nodes++;

	int eval = Evaluate::evaluate(pos);

	if (eval >= beta) return beta;
	if (eval > alpha) alpha = eval;

	MoveList all, moves;
	moveGen.genAllMoves(pos, all);

	for (int i = 0; i < all.size(); i++) {
		if (MoveUtils::getCapturePiece(all[i]) != EMPTY) 
			moves.push(all[i]);
	}
	scoreMoves(moves, MoveUtils::MOVE_NULL);

	for (int i = 0; i < moves.size(); i++) {

		int bestIndex = i;
		for (int j = i + 1; j < moves.size(); j++) {
			if (moves.scores[j] > moves.scores[bestIndex]) {
				bestIndex = j;
			}
		}

		std::swap(moves[i], moves[bestIndex]);
		std::swap(moves.scores[i], moves.scores[bestIndex]);

		pos.makeMove(moves[i]);

		if (pos.isInCheck(ENEMY)) {
			pos.unmakeMove(moves[i]);
			continue;
		}

		int score = -quiescence(pos, ss + 1, -beta, -alpha);
		pos.unmakeMove(moves[i]);

		if (this->stopSearch) return 0;

		if (score >= beta) return beta;
		if (score > alpha) alpha = score;
	}
	return alpha;
}


void Search::scoreMoves(MoveList& moves, Move firstMove) {
	for (int i = 0; i < moves.size(); i++) {


		Move m = moves[i];
		int score = 0;

		if (firstMove != 0 && moves[i] == firstMove) {
			moves.scores[i] = 1000000;
			continue;
		}
		int capturePiece = MoveUtils::getCapturePiece(m);
		int movePiece = MoveUtils::getMovePiece(m);

		if (capturePiece != EMPTY) { // Its a capture
			score = mvvLva[movePiece][capturePiece];
		}
		else { // score for quiet moves

		}

		int flags = MoveUtils::getFlags(m);
		if (flags == MoveFlag::PromoteQueen || flags == MoveFlag::PromoteCaptureQueen) {
			score += 80;
		}
		moves.scores[i] = score;
	}
}

