#include "Cell.h"
#include <stdexcept>
Cell::Cell(bool passable, int cost) : passable_(passable), cost_(cost) {
	if (cost < NormalMovementCost || cost > SlowMovementCost) {
		throw std::invalid_argument("Invalid cell cost");
	}
}

bool Cell::passable() const { return passable_; }

int Cell::cost() const { return cost_; }

int Cell::robotId() const { return robotId_; }

bool Cell::known() const { return discovered_; }

void Cell::setRobotId(int id) { robotId_ = id; }

void Cell::remember() { discovered_ = true; }
