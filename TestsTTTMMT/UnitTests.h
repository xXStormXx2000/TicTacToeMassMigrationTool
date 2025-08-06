#pragma once

#include "BaseTypes.h"
#include "BoardConverter.h"
#include "HuffmanTree.h"

BoardStream createRandomBoards(int numberOfBoards);

bool roundTripTest(const BoardStream& boards);