#pragma once
#include "Types.h"

enum TTFlag {
	FLAG_EXACT,
	FLAG_ALPHA, // Upper bound 
	FLAG_BETA // Lower Bound
};


struct TTEntry {
	U64 key;
	int score;
	int depth;
	TTFlag flag;
	Move bestMove;
};


const int TTSize = 1048576; // 1 MB 
extern TTEntry TT[TTSize];

