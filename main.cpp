#include "Position.h"
#include "MoveGenerator.h"

Position pos("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
MoveGenerator generator;

// Helper to convert piece constants to characters
char getPieceChar(int piece) {
	// This assumes 0 = Empty, Positive = White, Negative = Black
	// Adjust based on your specific enum/constants
	switch (piece) {
	case PAWN: return 'P'; case KNIGHT: return 'N'; case BISHOP: return 'B';
	case ROOK: return 'R'; case QUEEN: return 'Q'; case KING: return 'K';
	default: return '.'; // Empty square
	}
}

void printBoard() {
	printf("\n  +---+---+---+---+---+---+---+---+\n");

	// Chess boards are printed from Rank 8 down to Rank 1
	for (int rank = 7; rank >= 0; rank--) {
		printf("%d |", rank + 1); // Print rank number

		for (int file = 0; file < 8; file++) {
			// Calculate index: (rank * 8) + file for 64-slot array
			int square = rank * 8 + file;
			int piece = pos.pieceOnBoard(square);

			printf(" %d |", piece);//getPieceChar(piece));
		}
		printf("\n  +---+---+---+---+---+---+---+---+\n");
	}
	printf("    a   b   c   d   e   f   g   h\n\n");
}


void printMove(Move move) {
	// Get the raw integer squares
	int fromSq = MoveUtils::getFrom(move);
	int toSq = MoveUtils::getTo(move);
	int piece = MoveUtils::getMovePiece(move);

	// Calculate the characters for the algebraic notation
	char fromFile = 'a' + (fromSq % 8);
	char fromRank = '1' + (fromSq / 8);
	char toFile = 'a' + (toSq % 8);
	char toRank = '1' + (toSq / 8); 

	// 3. Print your move info ALONG WITH the branch nodes
	printf("%c%c%c%c: ", fromFile, fromRank, toFile, toRank);
}
U64 perft(int depth) {

	MoveList moves;
	U64 nodes = 0;

	if (depth == 0) {
		return C64(1);
	}

	generator.genAllMoves(pos, moves);

	for (int i = 0; i < moves.size(); i++) {
		//printMove(moves[i]);
		//printf("\n");
		pos.makeMove(moves[i]);
		
		if (!pos.isInCheck()) {
			nodes += perft(depth - 1);
		} 
		pos.unmakeMove(moves[i]);
	}

	return nodes;
}

void perftDivide(int depth) {
	if (depth == 0) {
		return;
	}

	MoveList moves;
	U64 totalNodes = 0; // Use U64 for the total to prevent overflow

	generator.genAllMoves(pos, moves);


	for (int i = 0; i < moves.size(); i++) {

		pos.makeMove(moves[i]);

		if (!pos.isInCheck()) {

			U64 branchNodes = perft(depth - 1);

			totalNodes += branchNodes;
			printMove(moves[i]);
			printf("%lld \n", branchNodes);
				
		}

		pos.unmakeMove(moves[i]);
	}

	printf("\nTotal Nodes: %llu\n", totalNodes);
}


int main(int argc, char* argv[]) {
	
	initMoveData();

	U64 expectedValues[] = {
		48, 2039, 97862, 4085603, 193690690, 8031647685};

	//U64 nodes = perft(3);
	//printf("%lld ", nodes);
	//perftDivide(2);

	for (int depth = 0; depth < 6; depth += 1) {
		U64 nodes = perft(depth + 1);
		printf("Depth: %d \t Expected Value = %lld, Real Value = %lld\n", depth + 1, expectedValues[depth], nodes);
	}
	return 0;
}





