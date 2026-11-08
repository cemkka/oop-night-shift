#include "Inventory.h"
#include "Robot.h"
#include <limits>
#include <stdexcept>

bool Inventory::valid(Item item) {
	int index = static_cast<int>(item);
	return index >= 0 && index < ItemCount;
}

std::string Inventory::name(Item item) {
	switch (item) {
	case Item::Health: return "Ремкомплект";
	case Item::Energy: return "Батарея";
	case Item::Attack: return "Усилитель";
	default: throw std::invalid_argument("Invalid item");
	}
}

std::string Inventory::description(Item item) {
	switch (item) {
	case Item::Health: return "+" + std::to_string(HealthRecovery) + " здоровья, не выше максимума";
	case Item::Energy: return "+" + std::to_string(EnergyRecovery) + " энергии, не выше максимума";
	case Item::Attack: return "+" + std::to_string(AttackIncrease) + " атаки и ремонта на "
		+ std::to_string(AttackDuration) + " собственных хода; повторное применение продлевает срок";
	default: throw std::invalid_argument("Invalid item");
	}
}

int Inventory::count(Item item) const {
	return valid(item) ? counts_[static_cast<int>(item)] : 0;
}

bool Inventory::add(Item item, int amount) {
	if (!valid(item) || amount <= 0) {
		return false;
	}
	int& quantity = counts_[static_cast<int>(item)];
	if (amount > std::numeric_limits<int>::max() - quantity) {
		return false;
	}
	quantity += amount;
	return true;
}

bool Inventory::use(Item item, Robot& robot) {
	if (!valid(item) || robot.inventory() != this || !robot.alive() || count(item) == 0) {
		return false;
	}
	switch (item) {
	case Item::Health:
		if (robot.health() == robot.maxHealth()) {
			return false;
		}
		robot.repair(HealthRecovery);
		break;
	case Item::Energy:
		if (robot.energy() == robot.maxEnergy()) {
			return false;
		}
		robot.restoreEnergy(EnergyRecovery);
		break;
	case Item::Attack:
		if (attackTurns_ > std::numeric_limits<int>::max() - AttackDuration
			|| (attackTurns_ == 0 && robot.attack() == std::numeric_limits<int>::max())) {
			return false;
		}
		attackTurns_ += AttackDuration;
		break;
	default: return false;
	}
	--counts_[static_cast<int>(item)];
	return true;
}

int Inventory::attackBonus() const { return attackTurns_ > 0 ? AttackIncrease : 0; }

int Inventory::attackTurns() const { return attackTurns_; }

void Inventory::beginTurn() { turnActive_ = true; }

void Inventory::finishTurn() {
	if (turnActive_ && attackTurns_ > 0) {
		--attackTurns_;
	}
	turnActive_ = false;
}
