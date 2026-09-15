#pragma once
#include "MoveGenerator.h"
#include <chrono>


struct SearchStack {
	int ply;
	Move currentMove;
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

	int negamax(Position& pos, SearchStack* ss, int depth, int alpha, int beta);
	int quiescence(Position& pos, SearchStack* ss, int alpha, int beta);
	void scoreMoves(MoveList& moves);
	
public: 
	Search() = default;
	Move getBestMove(Position& pos, short depth, long long remTime, long long incTime);
	inline long long getNodes() const { return nodes; }

};