#include "RobotFactory.h"
#include <stdexcept>
RobotFactory::RobotFactory(Field& field, int row, int column, int period)
	: field_(field), row_(row), column_(column), period_(period) {
	if (period <= 0) {
		throw std::invalid_argument("Invalid factory period");
	}
	field_.placeFactory(row, column);
}

bool RobotFactory::occupies(int row, int column) const {
	return row >= row_ && row < row_ + Size && column >= column_ && column < column_ + Size;
}

bool RobotFactory::adjacent(int row, int column) const {
	return ((row == row_ - Field::NeighborDistance || row == row_ + Size) && column >= column_ && column < column_ + Size)
		|| ((column == column_ - Field::NeighborDistance || column == column_ + Size) && row >= row_ && row < row_ + Size);
}

bool RobotFactory::active() const { return active_; }

bool RobotFactory::disable(int row, int column) {
	if (!active_ || !adjacent(row, column)) {
		return false;
	}
	active_ = false;
	return true;
}

bool RobotFactory::advance() {
	if (!active_ || ++elapsed_ < period_) {
		return false;
	}
	elapsed_ = 0;
	for (int row = row_ - Field::NeighborDistance; row <= row_ + Size; ++row) {
		for (int column = column_ - Field::NeighborDistance; column <= column_ + Size; ++column) {
			if (adjacent(row, column) && field_.free(row, column)) {
				field_.addRobot(Robot(Robot::Side::Enemy, EnemyHealth, EnemyAttack, EnemySpeed, EnemyVision), row, column);
				return true;
			}
		}
	}
	return false;
}