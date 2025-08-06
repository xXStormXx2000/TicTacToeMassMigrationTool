#include "TicTacToeMassMigrationTool.h"
#include "GameSimulator.h"
#include "UnitTests.h"


int main() {
	int gamesNum = 100'000;
	for (int i = 0; i <= 10; i++) {
		double epsilon = i / 10.0;
		std::cout << "Games: " << gamesNum << '\n';
		std::cout << "Epsilon: " << epsilon << '\n';
		std::cout << "Generating games...\n";
		GameList games = simulateGames(gamesNum, epsilon);
		std::cout << "Completed.\n\n";
		std::cout << "Expecting boards from games...\n";
		BoardStream boards = extractBoardsFromGames(games);
		std::cout << "Completed..\n\n";
		std::cout << "Boards: " << boards.size() << "\n\n";
		if (roundTripTest(boards)) {
			std::cout << "Round trip test was successful.\n\n\n";
		} else {
			std::cout << "Error: Round trip test was unsuccessful.\n\n\n";
		}
	}
	
	std::cout << "Random boards: " << gamesNum*10 << '\n';
	std::cout << "Generating random boards...\n";
	BoardStream randomBoards = createRandomBoards(gamesNum*10);
	std::cout << "Completed.\n\n";
	if (roundTripTest(randomBoards)) {
		std::cout << "Round trip test was successful.\n\n\n";
	}
	else {
		std::cout << "Error: Round trip test was unsuccessful.\n\n\n";
	}
}