#include "Field.h"
#include "RobotFactory.h"
#include <cstdlib>
#include <stdexcept>
#include <string>
Field::Field(int rows, int columns) : rows_(rows), columns_(columns) {
	if (rows < MinSize || rows > MaxSize || columns < MinSize || columns > MaxSize) {
		throw std::invalid_argument("Field dimensions must be "
			+ std::to_string(MinSize) + ".." + std::to_string(MaxSize));
	}
	cells_.resize(rows, std::vector<Cell>(columns));
	for (int row = 0; row < rows; ++row) {
		for (int column = 0; column < columns; ++column) {
			if (row == 0 || column == 0 || row == rows - 1 || column == columns - 1) {
				cells_[row][column] = Cell(false);
			}
		}
	}
}

int Field::rows() const { return rows_; }

int Field::columns() const { return columns_; }

bool Field::contains(int row, int column) const { return row >= 0 && column >= 0 && row < rows_ && column < columns_; }
const Cell& Field::cell(int row, int column) const { return cells_.at(row).at(column); }

void Field::setTerrain(int row, int column, bool passable, int cost) {
	if (cell(row, column).robotId() != Robot::NoRobotId || cell(row, column).terrainLocked_) {
		throw std::logic_error("Occupied or reserved cell");
	}
	cells_.at(row).at(column) = Cell(passable, cost);
}

bool Field::free(int row, int column) const {
	return contains(row, column) && cell(row, column).passable()
		&& cell(row, column).robotId() == Robot::NoRobotId;
}

int Field::addRobot(const Robot& value, int row, int column) {
	constexpr std::size_t CapacityGrowth = 2;
	if (!value.alive() || !free(row, column)) {
		throw std::invalid_argument("Invalid placement");
	}
	int id = robotCount();
	Robot copy = value;
	std::size_t requiredCapacity = robots_.size() + 1;
	if (robots_.capacity() < requiredCapacity || robotRows_.capacity() < requiredCapacity
		|| robotColumns_.capacity() < requiredCapacity) {
		std::size_t newCapacity = requiredCapacity * CapacityGrowth;
		robots_.reserve(newCapacity);
		robotRows_.reserve(newCapacity);
		robotColumns_.reserve(newCapacity);
	}
	robots_.push_back(copy);
	robotRows_.push_back(row);
	robotColumns_.push_back(column);
	cells_[row][column].setRobotId(id);
	return id;
}

int Field::robotCount() const { return static_cast<int>(robots_.size()); }

const Robot& Field::robot(int id) const { return robots_.at(id); }

bool Field::interact(int actor, int target) {
	if (distanceTo(actor, row(target), column(target)) != NeighborDistance) {
		return false;
	}
	bool success = robots_.at(actor).interact(robots_.at(target));
	removeDead();
	return success;
}

void Field::takeDamage(int id, int amount) {
	robots_.at(id).takeDamage(amount);
	removeDead();
}

bool Field::spendMovement(int id, int amount) { return robots_.at(id).spendMovement(amount); }

void Field::beginTurn(int id) { robots_.at(id).beginTurn(); }

void Field::finishTurn(int id) { robots_.at(id).finishTurn(); }

void Field::placeFactory(int topRow, int leftColumn) {
	if (topRow < 0 || leftColumn < 0
		|| topRow > rows_ - RobotFactory::Size || leftColumn > columns_ - RobotFactory::Size) {
		throw std::invalid_argument("Invalid factory coordinates");
	}
	for (int row = topRow; row < topRow + RobotFactory::Size; ++row) {
		for (int column = leftColumn; column < leftColumn + RobotFactory::Size; ++column) {
			if (!free(row, column)) {
				throw std::invalid_argument("Invalid factory placement");
			}
		}
	}
	for (int row = topRow; row < topRow + RobotFactory::Size; ++row) {
		for (int column = leftColumn; column < leftColumn + RobotFactory::Size; ++column) {
			cells_[row][column] = Cell(false);
			cells_[row][column].terrainLocked_ = true;
		}
	}
}

int Field::row(int id) const { return robotRows_.at(id); }

int Field::column(int id) const { return robotColumns_.at(id); }

bool Field::move(int id, int targetRow, int targetColumn) {
	if (!free(targetRow, targetColumn) || distanceTo(id, targetRow, targetColumn) != NeighborDistance) {
		return false;
	}
	if (!robots_.at(id).spendMovement(cell(targetRow, targetColumn).cost())) {
		return false;
	}
	cells_[row(id)][column(id)].setRobotId(Robot::NoRobotId);
	robotRows_[id] = targetRow;
	robotColumns_[id] = targetColumn;
	cells_[targetRow][targetColumn].setRobotId(id);
	return true;
}

void Field::removeDead() {
	for (int id = 0; id < robotCount(); ++id) {
		if (!robot(id).alive() && cells_[row(id)][column(id)].robotId() == id) {
			cells_[row(id)][column(id)].setRobotId(Robot::NoRobotId);
		}
	}
}

int Field::distanceTo(int id, int targetRow, int targetColumn) const {
	if (!contains(targetRow, targetColumn)) {
		throw std::out_of_range("Target outside field");
	}
	return std::abs(row(id) - targetRow) + std::abs(column(id) - targetColumn);
}

bool Field::visible(int id, int row, int column) const {
	return contains(row, column) && distanceTo(id, row, column) <= robot(id).vision();
}

void Field::remember(int id) {
	for (int row = 0; row < rows_; ++row) {
		for (int column = 0; column < columns_; ++column) {
			if (visible(id, row, column)) {
				cells_[row][column].remember();
			}
		}
	}
}

bool Field::hasEnemies() const {
	for (const Robot& value : robots_) {
		if (value.alive() && value.side() == Robot::Side::Enemy) {
			return true;
		}
	}
	return false;
}
