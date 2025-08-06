// Script generated with ChatGPT
#pragma once

#include "BaseTypes.h"
#include <vector>
#include <random>
#include <utility>
#include <algorithm>

// -----------------------------------------------------------------------------
// Tic-Tac-Toe self-play simulator with epsilon-greedy AI.
// -----------------------------------------------------------------------------
//  * If a winning move exists, it is always taken.
//  * With probability `epsilon` the AI plays a random legal move.
//  * Otherwise it chooses, in order of preference:
//        – Block opponent's immediate win
//        – Take centre
//        – Take a free corner
//        – Take any random legal move
//
// This yields realistic, non-optimal play well-suited for compression tests.
// -----------------------------------------------------------------------------

namespace detail {

    // Winning line descriptors: 3 indices per line encoded as {row,col} pairs
    static constexpr std::pair<int8_t, int8_t> kLines[8][3] = {
        /* rows    */ {{0,0},{0,1},{0,2}}, {{1,0},{1,1},{1,2}}, {{2,0},{2,1},{2,2}},
        /* columns */ {{0,0},{1,0},{2,0}}, {{0,1},{1,1},{2,1}}, {{0,2},{1,2},{2,2}},
        /* diagonals */ {{0,0},{1,1},{2,2}}, {{0,2},{1,1},{2,0}}
    };

    inline bool isWinner(const Board& b, Square player)
    {
        for (const auto& line : kLines) {
            if (b.squares[line[0].first][line[0].second] == player &&
                b.squares[line[1].first][line[1].second] == player &&
                b.squares[line[2].first][line[2].second] == player)
                return true;
        }
        return false;
    }

    inline bool boardFull(const Board& b)
    {
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                if (b.squares[r][c] == Square::none) return false;
        return true;
    }

    // Return list of empty squares as (row,col)
    inline std::vector<std::pair<int8_t, int8_t>> legalMoves(const Board& b)
    {
        std::vector<std::pair<int8_t, int8_t>> moves;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                if (b.squares[r][c] == Square::none) moves.emplace_back(r, c);
        return moves;
    }

    // If the given player can win in one move, return that move, else {-1,-1}
    inline std::pair<int8_t, int8_t> findImmediateWin(Board b, Square player)
    {
        for (const auto& mv : legalMoves(b)) {
            b.squares[mv.first][mv.second] = player;
            if (isWinner(b, player)) return mv;
            b.squares[mv.first][mv.second] = Square::none; // undo
        }
        return { -1,-1 };
    }

} // namespace detail

// -----------------------------------------------------------------------------
// Simulate a single game. Returns a Game object containing every board state
// after each move (starting from the first move, excluding the initial empty
// board).  Epsilon defaults to 0.2 (20 % random play).
// -----------------------------------------------------------------------------
inline Game simulateGame(double epsilon = 0.2)
{
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<double> uni(0.0, 1.0);

    Game game;
    Board board; // starts empty (all Square::none)
    Square current = Square::X;

    while (true)
    {
        using namespace detail;
        auto moves = legalMoves(board);
        if (moves.empty()) break; // draw

        std::pair<int8_t, int8_t> chosen{ -1,-1 };

        // 1. Winning move?
        chosen = findImmediateWin(board, current);

        // 2. Block opponent's win
        if (chosen.first == -1)
            chosen = findImmediateWin(board, current == Square::X ? Square::O : Square::X);

        // 3. Epsilon-greedy randomisation
        if (chosen.first == -1 && uni(rng) < epsilon)
            chosen = moves[rng() % moves.size()];

        // 4. Centre
        if (chosen.first == -1 && board.squares[1][1] == Square::none)
            chosen = { 1,1 };

        // 5. Corner preference
        static constexpr std::pair<int8_t, int8_t> corners[4] = { {0,0},{0,2},{2,0},{2,2} };
        if (chosen.first == -1) {
            std::vector<std::pair<int8_t, int8_t>> freeCorners;
            for (auto c : corners)
                if (board.squares[c.first][c.second] == Square::none) freeCorners.push_back(c);
            if (!freeCorners.empty()) chosen = freeCorners[rng() % freeCorners.size()];
        }

        // 6. Fallback random
        if (chosen.first == -1)
            chosen = moves[rng() % moves.size()];

        // Apply move
        board.squares[chosen.first][chosen.second] = current;
        game.boards.push_back(board);

        // Check terminal states
        if (detail::isWinner(board, current) || detail::boardFull(board)) break;

        // Switch player
        current = (current == Square::X ? Square::O : Square::X);
    }
    return game;
}

// -----------------------------------------------------------------------------
// Simulate many games and return as a GameList.
// -----------------------------------------------------------------------------
inline GameList simulateGames(size_t count, double epsilon = 0.2)
{
    GameList games;
    games.reserve(count);
    for (size_t i = 0; i < count; ++i)
        games.push_back(simulateGame(epsilon));
    return games;
}

