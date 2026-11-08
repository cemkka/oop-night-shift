#include "statuses/Burning.h"
#include "Robot.h"
#include <stdexcept>

Burning::Burning(int turns) : turns_(turns) {
	if (turns <= 0) {
		throw std::invalid_argument("Invalid burning duration");
	}
}

std::string Burning::name() const { return "Горение (" + std::to_string(turns_) + ")"; }

std::unique_ptr<Status> Burning::clone() const { return std::make_unique<Burning>(*this); }

bool Burning::merge(const Status& other) {
	const auto* burning = dynamic_cast<const Burning*>(&other);
	if (!burning) {
		return false;
	}
	turns_ = saturatedAdd(turns_, burning->turns_);
	return true;
}

bool Burning::expired() const { return turns_ == 0; }

void Burning::finishTurn(Robot& robot) {
	if (turns_ > 0 && robot.alive()) {
		robot.takeDamage(Damage);
		--turns_;
	}
}
