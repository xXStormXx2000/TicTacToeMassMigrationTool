#pragma once

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <queue>
#include <string>
#include <iostream>

#include "BaseTypes.h"

class HuffmanTree {
	struct Node {
		uint16_t value = uint16_t(-1);

		std::shared_ptr<Node> parent = nullptr;
		std::shared_ptr<Node> childOne = nullptr;
		std::shared_ptr<Node> childTwo = nullptr;
	};
	std::unordered_map<uint16_t, std::shared_ptr <Node>> leafNodes;
	std::shared_ptr<Node> head = nullptr;
	uint16_t getBoardAtPos(const uint8_t* data, size_t bytes, size_t bitPos);
	void writeBoardAtPos(ByteVector& data, size_t bitPos, uint16_t board, size_t size);

	void DFSC(std::shared_ptr<Node> node, ByteVector& data, size_t& bitPos);
	bool DFSE(const std::shared_ptr<Node> node, const std::shared_ptr<Node> otherNode) const;
public:
	HuffmanTree(const ByteVector& raw);
	HuffmanTree(const std::uint8_t* raw, size_t byteCount);

	bool operator==(const HuffmanTree& other) const;

	void serialize(ByteVector& raw);
	ByteVector deserialization(const std::uint8_t* raw, size_t byteCount, size_t boardCount);
	ByteVector getHuffmanTree();
};

