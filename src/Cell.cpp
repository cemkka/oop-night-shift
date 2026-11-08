#include "Cell.h"
#include <stdexcept>
#include <utility>
Cell::Cell(bool passable, int cost) : passable_(passable), cost_(cost) {
	if (cost < NormalMovementCost || cost > SlowMovementCost) {
		throw std::invalid_argument("Invalid cell cost");
	}
}

bool Cell::passable() const { return passable_; }

int Cell::statusCount() const { return static_cast<int>(statuses_.size()); }

const Status& Cell::status(int index) const { return *statuses_.at(index); }

void Cell::addStatus(std::unique_ptr<Status> value) {
	if (!value || !passable_ || statusCount() >= MaxStatuses) {
		throw std::invalid_argument("Invalid cell status");
	}
	statuses_.push_back(std::move(value));
}

int Cell::cost() const { return cost_; }

int Cell::robotId() const { return robotId_; }

bool Cell::known() const { return discovered_; }

void Cell::setRobotId(int id) { robotId_ = id; }

void Cell::remember() { discovered_ = true; }
