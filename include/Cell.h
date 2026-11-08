#pragma once
#include "Robot.h"

class Cell {
public:
	static constexpr int NormalMovementCost = 1;
	static constexpr int SlowMovementCost = 2;
	static constexpr int MaxStatuses = 2;

	Cell(bool passable = true, int cost = NormalMovementCost);
	Cell(const Cell&) = delete;
	Cell& operator=(const Cell&) = delete;
	Cell(Cell&&) noexcept = default;
	Cell& operator=(Cell&&) noexcept = default;
	int statusCount() const;
	const Status& status(int index) const;
	void addStatus(std::unique_ptr<Status> status);
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
	std::vector<std::unique_ptr<Status>> statuses_;
};
