#include "board/Position.h"
#include "search/Search.h"
#include "core/Zobrist.h"
#include "search/TT.h"

#include <climits>
#include <sstream>
#include <string>
#include <iostream>
#include <memory>


// Helper to convert piece constants to characters

// Helper to convert "e2e4" into your internal Move integer
Move parseUciMove(Position& pos, std::string moveStr) {
	MoveList list;
	MoveGenerator moveGen;
	moveGen.genAllMoves(pos, list);

	for (int i = 0; i < list.size(); i++) {
		Move m = list[i];
		if (MoveUtils::printMove(m) == moveStr) {
			return m;
		}
	}
	return 0; // Invalid move
}
void uciLoop() {
	Search engineBrain;
	std::unique_ptr<Position> pos = std::make_unique<Position>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	initMoveData();

	std::string line;

	while (std::getline(std::cin, line)) {
		std::stringstream ss(line);
		std::string token;
		ss >> token;

		if (token == "uci") {
			std::cout << "uciok" << '\n';
		}
		else if (token == "isready") {
			std::cout << "readyok" << '\n';
		}
		else if (token == "ucinewgame") {
			engineBrain.newGame();
		}
		else if (token == "position") {
			std::string type;

			ss >> type;
			if (type == "startpos") {
				pos = std::make_unique<Position>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			}
			else if (type == "fen") {
				std::string fen = "";
				for (int i = 0; i < 6; i++) {
					std::string f;
					ss >> f;
					fen += f + " ";
				}
				pos = std::make_unique<Position>(fen);
			}

			std::string next;

			ss >> next;
			if (next == "moves") {
				std::string moveStr;
				while (ss >> moveStr) {
					Move m = parseUciMove(*pos, moveStr);
					pos -> makeMove(m);
				}
			}
		}
		else if (token == "go") {
			short depth = 64;
			long long wtime = INT_MAX, btime = INT_MAX, winc = 0, binc = 0;
			std::string param;
			while (ss >> param) {
				if (param == "depth") ss >> depth;
				if (param == "wtime") ss >> wtime;
				if (param == "btime") ss >> btime;
				if (param == "winc")  ss >> winc;
				if (param == "binc")  ss >> binc;
  			}

			Color turn = pos->getSideToMove();
			Move bestMove;

			if(turn == WHITE) 
				bestMove = engineBrain.getBestMove(*pos, depth, wtime, winc);
			else 
				bestMove = engineBrain.getBestMove(*pos, depth, btime, binc);

			std::cout << "bestmove " << MoveUtils::printMove(bestMove) << "\n";
		}
		else if (token == "quit") {
			break;
		}
	}
}
int main(int argc, char* argv[]) {
	std::setvbuf(stdout, NULL, _IONBF, 0);

	zobrist.init();

	uciLoop();
	return 0;
}





