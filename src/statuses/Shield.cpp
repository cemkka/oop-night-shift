#include "statuses/Shield.h"
#include <stdexcept>

Shield::Shield(int protection) : protection_(protection) {
	if (protection <= 0) {
		throw std::invalid_argument("Invalid shield protection");
	}
}

std::string Shield::name() const { return "Щит (" + std::to_string(protection_) + ")"; }

std::unique_ptr<Status> Shield::clone() const { return std::make_unique<Shield>(*this); }

bool Shield::merge(const Status& other) {
	const auto* shield = dynamic_cast<const Shield*>(&other);
	if (!shield) {
		return false;
	}
	protection_ = saturatedAdd(protection_, shield->protection_);
	return true;
}

bool Shield::expired() const { return protection_ == 0; }

int Shield::absorbDamage(int amount) {
	if (amount <= protection_) {
		return 0;
	}
	int remaining = amount - protection_;
	protection_ = 0;
	return remaining;
}
