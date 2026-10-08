#pragma once
#include "board/Position.h"
#include "core/BitboardUtils.h"
#include "core/Move.h"


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

