#include "TicTacToeMassMigrationTool.h"

void streamOutBoards(const BoardStream& boards, std::string IP, size_t port) {
	ByteVector memory = boardsToMemoryBlock(boards);
	HuffmanTree tree(memory);
	ByteVector treeMemory = tree.getHuffmanTree();
	tree.serialize(memory);
	ByteVector outData;

	//Header
	outData.insert(outData.begin(), {
			uint8_t(treeMemory.size() >> 8 * 0), uint8_t(treeMemory.size() >> 8 * 1), uint8_t(treeMemory.size() >> 8 * 2), uint8_t(treeMemory.size() >> 8 * 3), uint8_t(treeMemory.size() >> 8 * 4), uint8_t(treeMemory.size() >> 8 * 5), uint8_t(treeMemory.size() >> 8 * 6), uint8_t(treeMemory.size() >> 8 * 7),
			uint8_t(memory.size() >> 8 * 0), uint8_t(memory.size() >> 8 * 1), uint8_t(memory.size() >> 8 * 2), uint8_t(memory.size() >> 8 * 3), uint8_t(memory.size() >> 8 * 4), uint8_t(memory.size() >> 8 * 5), uint8_t(memory.size() >> 8 * 6), uint8_t(memory.size() >> 8 * 7),
			uint8_t(boards.size() >> 8 * 0), uint8_t(boards.size() >> 8 * 1), uint8_t(boards.size() >> 8 * 2), uint8_t(boards.size() >> 8 * 3), uint8_t(boards.size() >> 8 * 4), uint8_t(boards.size() >> 8 * 5), uint8_t(boards.size() >> 8 * 6), uint8_t(boards.size() >> 8 * 7)
		});

	//Data
	outData.insert(outData.end(), treeMemory.begin(), treeMemory.end());
	outData.insert(outData.end(), memory.begin(), memory.end());
	sendData(IP, port, outData);
}

BoardStream streamInBoards(std::string IP, size_t port) {
	ByteVector inData = getData(IP, port);

	if (inData.size() < 24) return {};
	size_t treeBytes = inData[0] << 8 * 0 | inData[1] << 8 * 0 | inData[2] << 8 * 0 | inData[3] << 8 * 0 | inData[4] << 8 * 0 | inData[5] << 8 * 0 | inData[6] << 8 * 0 | inData[7] << 8 * 0;
	size_t memBytes = inData[8] << 8 * 0 | inData[9] << 8 * 0 | inData[10] << 8 * 0 | inData[11] << 8 * 0 | inData[12] << 8 * 0 | inData[13] << 8 * 0 | inData[14] << 8 * 0 | inData[15] << 8 * 0;
	size_t boards = inData[16] << 8 * 0 | inData[17] << 8 * 0 | inData[18] << 8 * 0 | inData[19] << 8 * 0 | inData[20] << 8 * 0 | inData[21] << 8 * 0 | inData[22] << 8 * 0 | inData[23] << 8 * 0;

	if (inData.size() != 24 + treeBytes + memBytes) return {};
	HuffmanTree tree(inData.data() + 24, treeBytes);
	ByteVector mem = tree.deserialization(inData.data() + 24 + treeBytes, memBytes, boards);

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
