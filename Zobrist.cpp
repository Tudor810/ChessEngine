#include "Zobrist.h"
#include <random>

ZobristKeys zobrist;

void ZobristKeys::init() {
	std::mt19937_64 rng(123456789);

	for (int p = 0; p < 12; p++) {
		for (int sq = 0; sq < 64; sq++) {
			zobrist.pieces[p][sq] = rng();
		}
	}
	zobrist.side = rng();
	for (int i = 0; i < 16; i++) zobrist.castling[i] = rng();
	for (int i = 0; i < 65; i++) zobrist.ep[i] = rng();
}