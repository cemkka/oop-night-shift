#pragma once
#include "Game.h"

class TestSuite {
public:
	static void run();
private:
	static void ownership();
	static void playerAbilities();
	static void commandsAndRewards();
	static void enemies();
	static void streams();
	static void attacks();
	static void statuses();
	static void cells();
	static void endings();
	static std::string runGame(Game& game, const std::string& input);
};
