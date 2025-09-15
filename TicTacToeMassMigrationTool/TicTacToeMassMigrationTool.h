#pragma once

#include <vector>
#include <string>

#include "BaseTypes.h"
#include "NetworkStreamHandler.h"
#include "BoardConverter.h"
#include "HuffmanTree.h"

void streamOutBoards(const BoardStream& boards, std::string IP, size_t port);
BoardStream streamInBoards(std::string IP, size_t port);

BoardStream extractBoardsFromGames(const GameList& games);
GameList reconstructGamesFromBoards(const BoardStream& boards);