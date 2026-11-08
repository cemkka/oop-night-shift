#include "statuses/Overload.h"

std::string Overload::name() const { return "Перегрузка"; }

std::unique_ptr<Status> Overload::clone() const { return std::make_unique<Overload>(*this); }

bool Overload::merge(const Status& other) { return dynamic_cast<const Overload*>(&other) != nullptr; }

bool Overload::expired() const { return !active_; }

bool Overload::cancelsAbility() {
	bool result = active_;
	active_ = false;
	return result;
}
