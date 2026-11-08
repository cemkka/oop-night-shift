#pragma once
#include "Cell.h"
#include "Robot.h"
#include <vector>
#include <random>

class Field {
public:
	static constexpr int NeighborDistance = 1;

	Field(int rows, int columns, bool populateStatuses = true);
	Field(const Field&) = delete;
	Field& operator=(const Field&) = delete;
	Field(Field&&) = delete;
	Field& operator=(Field&&) = delete;
	int rows() const;
	int columns() const;
	bool contains(int row, int column) const;
	const Cell& cell(int row, int column) const;
	void setTerrain(int row, int column, bool passable, int cost = Cell::NormalMovementCost);
	int addRobot(Robot robot, int row, int column);
	void addAbility(int id, std::unique_ptr<Ability> ability);
	void upgradeAbility(int id, int index);
	bool applyAbility(int id, int index, int row, int column);
	bool prepareAbility(int id);
	bool useItem(int id, Inventory::Item item);
	void addStatus(int id, std::unique_ptr<Status> status);
	void damageFrom(int actor, int target, int amount);
	void addCellStatus(int row, int column, std::unique_ptr<Status> status);
	int robotCount() const;
	const Robot& robot(int id) const;
	bool interact(int actor, int target);
	void takeDamage(int id, int amount);
	void repair(int id, int amount);
	bool spendEnergy(int id, int amount);
	bool spendMovement(int id, int amount);
	void beginTurn(int id);
	void finishTurn(int id);
	int row(int id) const;
	int column(int id) const;
	bool free(int row, int column) const;
	bool move(int id, int row, int column);
	bool teleport(int id, int row, int column);
	void removeDead();
	bool visible(int observer, int row, int column) const;
	int distanceTo(int observer, int targetRow, int targetColumn) const;
	void remember(int observer);
	bool hasEnemies() const;
private:
	static constexpr int MinSize = 8;
	static constexpr int MaxSize = 30;
	static constexpr int EmptyStatusWeight = 70;
	static constexpr int SingleStatusWeight = 25;
	static constexpr int DoubleStatusWeight = 5;
	static constexpr int CellBurningDuration = 3;
	static constexpr int CellShieldProtection = 10;

	friend class RobotFactory;
	void placeFactory(int row, int column);
	void enterCell(int id);
	void relocate(int id, int row, int column);
	int rows_, columns_;
	std::vector<std::vector<Cell>> cells_;
	// Индекс робота остаётся его ID: элементы не удаляются и не переставляются.
	std::vector<Robot> robots_;
	std::vector<int> robotRows_, robotColumns_;
	std::mt19937 generator_;
};
