#pragma once
#include "ConsoleRenderer.h"
#include <string>

class Game {
public:
	Game();
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
	enum class InputMode { InitialAbility, Command, Ability, Target, RankReward };

	friend class TestSuite;
	Field field_;
	RobotFactory factory_;
	ConsoleRenderer renderer_;
	int turn_ = FirstTurn;
	InputMode inputMode_ = InputMode::InitialAbility;
	int selectedAbility_ = 0;
	int rewardedRank_ = 1;
	bool endTurnPending_ = false;
	std::string message_ = "Отключите станок и обезвредьте все взбунтовавшиеся экспонаты.";
	bool step(int id, int row, int column);
	void computerTurn();
	void finishTurn();
	void enemyAbility(int id);
	int findAbility(bool teleportation) const;
	bool chooseAbility(const std::string& input);
	bool useAbility(const std::string& input);
	void applyPlayerAbility(int row, int column);
	void prompt() const;
	static bool parseNumber(const std::string& input, int& number);
};
