#pragma once
#include "Position.h"
#include "BitboardUtils.h"
#include "Move.h"


class MoveGenerator {
	
	private: 
		void genPawnMoves(const Position& pos, MoveList& list);
		void genKnightMoves(const Position& pos, MoveList& list);
		void genBishopMoves(const Position& pos, MoveList& list);
		void genRookMoves(const Position& pos, MoveList& list);
		void genQueenMoves(const Position& pos, MoveList& list);
		void genKingMoves(const Position& pos, MoveList& list);
	public:
		void genAllMoves(const Position& pos, MoveList& list);
};

