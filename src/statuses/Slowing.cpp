#include "statuses/Slowing.h"
#include <stdexcept>

Slowing::Slowing(int turns) : turns_(turns) {
	if (turns <= 0) {
		throw std::invalid_argument("Invalid slowing duration");
	}
}

std::string Slowing::name() const { return "Замедление (" + std::to_string(turns_) + ")"; }

std::unique_ptr<Status> Slowing::clone() const { return std::make_unique<Slowing>(*this); }

bool Slowing::merge(const Status& other) {
	const auto* slowing = dynamic_cast<const Slowing*>(&other);
	if (!slowing) {
		return false;
	}
	turns_ = saturatedAdd(turns_, slowing->turns_);
	return true;
}

bool Slowing::expired() const { return turns_ == 0; }

bool Slowing::blocksMovement() const { return !expired(); }

void Slowing::beginTurn() { affectedTurn_ = true; }

void Slowing::finishTurn(Robot&) {
	if (affectedTurn_ && turns_ > 0) {
		--turns_;
	}
	affectedTurn_ = false;
}
