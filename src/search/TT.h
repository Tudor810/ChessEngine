#pragma once
#include "core/Types.h"

enum TTFlag : uint8_t {
	FLAG_EXACT,
	FLAG_ALPHA, // Upper bound 
	FLAG_BETA // Lower Bound
};


struct TTEntry {
	uint32_t key;
	int16_t score;
	uint8_t depth;
	TTFlag flag; 
	//uint8_t age;
	Move bestMove;
};

const int TTSize = 2097152; // 24 MB
extern TTEntry TT[TTSize];

