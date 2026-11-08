#pragma once
#include "ConsoleRenderer.h"
#include "AbilityMenu.h"
#include "InventoryMenu.h"
#include <string>

class Game {
public:
	explicit Game(bool populateStatuses = true);
	void run();
	bool command(const std::string& input);
	bool won() const;
	bool lost() const;
	const Field& field() const;
	const RobotFactory& factory() const;
	int turn() const;
private:
	static constexpr int MapRows = 10;
	static constexpr int MapColumns = 10;
	static constexpr int FactoryPeriod = 5;
	static constexpr int AllyHealth = 80;
	static constexpr int AllyAttack = 15;
	static constexpr int AllySpeed = 1;
	static constexpr int AllyVision = 4;
	static constexpr int EnemyHealth = 40;
	static constexpr int EnemyAttack = 6;
	static constexpr int EnemySpeed = 1;
	static constexpr int EnemyVision = 12;
	static constexpr int FirstTurn = 1;
	static constexpr int ShutdownCost = 1;
	static constexpr int CommandLength = 1;

	friend class TestSuite;
	Field field_;
	RobotFactory factory_;
	ConsoleRenderer renderer_;
	AbilityMenu menu_;
	InventoryMenu inventoryMenu_;
	ConsoleRenderer::Mode displayMode_ = ConsoleRenderer::Mode::Standard;
	int turn_ = FirstTurn;
	bool endTurnPending_ = false;
	std::string message_ = "Отключите станок и обезвредьте все взбунтовавшиеся экспонаты.";
	bool step(int id, int row, int column);
	void computerTurn();
	void finishTurn();
};
