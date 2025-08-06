#include "TicTacToeMassMigrationTool.h"

void streamOutBoards(const BoardStream& boards) {
	ByteVector memory = boardsToMemoryBlock(boards);
	HuffmanTree tree(memory);
	ByteVector treeMemory = tree.getHuffmanTree();
	tree.serialize(memory);
}

BoardStream streamInBoards() {
	uint8_t* treeMemory = nullptr;
	size_t treeBytes = 0;
	uint8_t* memory = nullptr;
	size_t bytes = 0;
	size_t boards = 0;
	
	HuffmanTree tree(treeMemory, treeBytes);
	delete[] treeMemory;
	ByteVector mem = tree.deserialization(memory, bytes, boards);
	delete[] memory;
	return memoryBlockToBoards(mem.data(), mem.size(), boards);
}

BoardStream extractBoardsFromGames(const GameList& games) {
	BoardStream boards;
	for (const Game& game: games) {
		for (const Board& board : game.boards) {
			boards.push_back(board);
		}
	}
	return boards;
}

GameList reconstructGamesFromBoards(const BoardStream& boards) {
	GameList games;
	Game game;
	for (const Board& board : boards) {
		int count = 0;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				count += (board.squares[i][j] != none);
				if (count > 1) break;
			}
			if (count > 1) break;
		}
		if (count > 1) {
			game.boards.push_back(board);
		} else if(count == 1) {
			if(!game.boards.empty()) games.push_back(game);
			game = Game{};
			game.boards.push_back(board);
		} else {
			throw (std::string)"A board is empty";
		}
	}
	if (!game.boards.empty()) games.push_back(game);
	return games;
}
