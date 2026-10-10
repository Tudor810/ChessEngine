#pragma once
#include "movegen/MoveGenerator.h"
#include <chrono>


struct SearchStack {
	int ply;
	Move currentMove;
	Move killers[2];
};

class Search {
private: 
	MoveGenerator moveGen;
	long long nodes = 0;
	
	// Time Management
	bool stopSearch = false;
	std::chrono::time_point<std::chrono::steady_clock> deadline;

	// Move Stack 
	
	static constexpr int MAX_PLY = 256;
	SearchStack stack[MAX_PLY] = {};
	int history[2][64][64] = {}; // history[color][from][to] = nr. of beta cuttof
	int negamax(Position& pos, SearchStack* ss, int depth, int alpha, int beta);
	int quiescence(Position& pos, SearchStack* ss, int alpha, int beta);
	void scoreMoves(MoveList& moves, Move firstMove, const SearchStack* ss, Color stm);
	void updateHistory(Color c, Move m, int bonus);
public: 
	Search() = default;
	Move getBestMove(Position& pos, short depth, long long remTime, long long incTime);
	inline long long getNodes() const { return nodes; }
	void newGame();

};