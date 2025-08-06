#include "UnitTests.h"

BoardStream createRandomBoards(int numberOfBoards) {
	BoardStream boards;
	for (int i = 0; i < numberOfBoards; i++) {
		Board board;
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {
				board.squares[y][x] = (Square)(rand() % 3);
			}
		}
		boards.push_back(board);
	}
	return boards;
}


bool roundTripTest(const BoardStream& boards) {
	std::cout << "Original size: " << boards.size() * sizeof(Board) << " Bytes.\n";
	double originalBits = boards.size() * sizeof(Board) * 8.0;
	ByteVector memory = boardsToMemoryBlock(boards);

	std::cout << "Packed size: " << memory.size() << " Bytes.\n";

	HuffmanTree tree(memory);
	ByteVector treeMemory = tree.getHuffmanTree();
	
	tree.serialize(memory);

	std::cout << "Huffman Tree size: " << treeMemory.size() << " Bytes.\n";
	std::cout << "Packed and serialized size: " << memory.size() << " Bytes.\n";
	double compressedBits = memory.size() * 8.0;
	
	std::cout << "Compression ratio: " << ((1 - compressedBits / originalBits) * 100.0) << "%\n\n";

	HuffmanTree newTree(treeMemory.data(), treeMemory.size());
	ByteVector mem = newTree.deserialization(memory.data(), memory.size(), boards.size());

	BoardStream recreatedBoards = memoryBlockToBoards(mem.data(), mem.size(), boards.size());

	if (boards.size() != recreatedBoards.size()) return false;

	for (size_t i = 0; i < boards.size() && i < recreatedBoards.size(); i++) {
		if (memcmp(&boards[i], &recreatedBoards[i], sizeof(Board)) != 0) {
			std::cout << i << ":\n";
			for (int y = 0; y < 3; y++) {
				for (int x = 0; x < 3; x++) {
					std::cout << "|" << (uint16_t)boards[i].squares[y][x] << "-" << (uint16_t)recreatedBoards[i].squares[y][x];
				}
				std::cout << "|\n";
			}
			std::cout << '\n';
			return false;
		}
	}
	return true;
}