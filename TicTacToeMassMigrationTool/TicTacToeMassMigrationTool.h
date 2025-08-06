#pragma once

#include <vector>
#include <string>

#include "BaseTypes.h"
#include "NetworkStreamHandler.h"
#include "BoardConverter.h"
#include "HuffmanTree.h"

void streamOutBoards(const BoardStream& boards);
BoardStream streamInBoards();

BoardStream extractBoardsFromGames(const GameList& games);
GameList reconstructGamesFromBoards(const BoardStream& boards);