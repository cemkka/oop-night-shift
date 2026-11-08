#pragma once
#include "Field.h"

class RobotFactory {
public:
	static constexpr int Size = 2;

	RobotFactory(Field& field, int row, int column, int period);
	RobotFactory(const RobotFactory&) = delete;
	RobotFactory& operator=(const RobotFactory&) = delete;
	RobotFactory(RobotFactory&&) = delete;
	RobotFactory& operator=(RobotFactory&&) = delete;
	bool occupies(int row, int column) const;
	bool adjacent(int row, int column) const;
	bool active() const;
	bool disable(int row, int column);
	bool advance();
private:
	static constexpr int EnemyHealth = 30;
	static constexpr int EnemyAttack = 5;
	static constexpr int EnemySpeed = 1;
	static constexpr int EnemyVision = 12;

	Field& field_;
	int row_, column_, period_, elapsed_ = 0;
	bool active_ = true;
};
