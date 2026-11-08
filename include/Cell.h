#pragma once
#include "Robot.h"

class Cell {
public:
	static constexpr int NormalMovementCost = 1;
	static constexpr int SlowMovementCost = 2;

	Cell(bool passable = true, int cost = NormalMovementCost);
	bool passable() const;
	int cost() const;
	int robotId() const;
	bool known() const;
	void remember();
private:
	friend class Field;
	void setRobotId(int id);
	bool terrainLocked_ = false;
	bool passable_, discovered_ = false;
	int cost_, robotId_ = Robot::NoRobotId;
};
