#pragma once
#include "BaseTypes.h"
#include <string>
#include <iostream>

namespace detail {
    constexpr uint8_t META_START = 0b1'000;
    constexpr uint8_t PATTERN_EMPTY = 0b10'001;
}

ByteVector boardsToMemoryBlock(const BoardStream& boards);
BoardStream memoryBlockToBoards(const std::uint8_t* data, size_t byteCount, size_t boardCount);