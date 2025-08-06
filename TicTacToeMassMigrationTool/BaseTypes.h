#pragma once

#include <vector>

enum Square : unsigned char {
    none,
    X,
    O
};

struct Board {
    Square squares[3][3] = {
        { none, none, none },
        { none, none, none },
        { none, none, none }
    };
};

struct Game {
    std::vector<Board> boards;
};

using BoardStream = std::vector<Board>;
using GameList = std::vector<Game>;

using ByteVector = std::vector<std::uint8_t>;