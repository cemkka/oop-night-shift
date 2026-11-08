#pragma once
#include <array>
#include <string>

class Robot;

class Inventory {
public:
	enum class Item { Health, Energy, Attack, Count };
	static constexpr int ItemCount = static_cast<int>(Item::Count);
	static constexpr int HealthRecovery = 30;
	static constexpr int EnergyRecovery = 5;
	static constexpr int AttackIncrease = 10;
	static constexpr int AttackDuration = 3;
	static bool valid(Item item);
	static std::string name(Item item);
	static std::string description(Item item);
	int count(Item item) const;
	bool add(Item item, int amount = 1);
	bool use(Item item, Robot& robot);
	int attackBonus() const;
	int attackTurns() const;
	void beginTurn();
	void finishTurn();
private:
	std::array<int, ItemCount> counts_{};
	int attackTurns_ = 0;
	bool turnActive_ = true;
};
