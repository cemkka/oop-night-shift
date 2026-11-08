#include "statuses/Burning.h"
#include "Robot.h"
#include "Field.h"
#include <stdexcept>

Burning::Burning(int turns, int sourceId) : turns_(turns), sourceId_(sourceId) {
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
	sourceId_ = burning->sourceId_;
	return true;
}

bool Burning::expired() const { return turns_ == 0; }

void Burning::finishTurn(Robot& robot, Field* field, int id) {
	if (turns_ > 0 && robot.alive()) {
		if (field && id >= 0 && id < field->robotCount()
			&& sourceId_ >= 0 && sourceId_ < field->robotCount() && field->robot(sourceId_).alive()) {
			field->damageFrom(sourceId_, id, Damage);
		} else {
			robot.takeDamage(Damage);
		}
		--turns_;
	}
}
