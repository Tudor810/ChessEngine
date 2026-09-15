#include "Search.h"
#include "Evaluate.h"
//#include <iostream> // Added for debugging



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
}

Move Search::getBestMove(Position& pos, short depth, long long remTime, long long incTime) {


	// Initialization

	MoveList moves;
	moveGen.genAllMoves(pos, moves);
	nodes = 0;
	this->stopSearch = false;
	
	SearchStack* ss = &this->stack[0];
	ss->ply = 0;

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


	scoreMoves(legalRootMoves);
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

	for (int d = 1; d <= depth; d++) {

		int alpha = -SHRT_MAX / 2;
		int beta = SHRT_MAX / 2;
		Move bestMoveThisDepth = 0;

		for (int i = 0; i < legalRootMoves.size(); i++) {

			

			pos.makeMove(legalRootMoves[i]);
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


	MoveList moves;
	moveGen.genAllMoves(pos, moves);

	scoreMoves(moves);

	int legalMoves = 0;

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

		int score = -negamax(pos, ss + 1, depth - 1, -beta, -alpha);
		pos.unmakeMove(moves[i]);

		if (this->stopSearch) {
			return 0;
		}
		if (score >= beta) return beta;
		if (score > alpha) alpha = score;
	}

	if (legalMoves == 0) {
		if (pos.isInCheck(ALLY)) { return VALUE_MATED + ss->ply; } // Checkmate
		
		return 0; // Stalemate
	}

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

	MoveList moves;
	moveGen.genAllMoves(pos, moves);
	
	// sortMoves(moves) - implement for faster pruning


	for (int i = 0; i < moves.size(); i++) {

		int bestIndex = i;
		for (int j = i + 1; j < moves.size(); j++) {
			if (moves.scores[j] > moves.scores[bestIndex]) {
				bestIndex = j;
			}
		}

		std::swap(moves[i], moves[bestIndex]);
		std::swap(moves.scores[i], moves.scores[bestIndex]);

		if (MoveUtils::getCapturePiece(moves[i]) == EMPTY) continue;

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


void Search::scoreMoves(MoveList& moves) {
	for (int i = 0; i < moves.size(); i++) {
		Move m = moves[i];
		int score = 0;

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