#include "abilities/Healing.h"
#include "Field.h"
#include <algorithm>
#include <limits>

std::string Healing::name() const { return "Лечение"; }

Ability::Kind Healing::kind() const { return Kind::Healing; }

bool Healing::canUpgrade() const { return healing_ < std::numeric_limits<int>::max(); }

int Healing::energyCost() const { return EnergyCost; }

bool Healing::apply(Field& field, int robotId, int, int) {
	if (!field.prepareAbility(robotId)) {
		return false;
	}
	const Robot& robot = field.robot(robotId);
	if (!robot.alive() || robot.health() == robot.maxHealth() || robot.energy() < energyCost()) {
		return false;
	}
	if (!field.spendEnergy(robotId, energyCost())) {
		return false;
	}
	field.repair(robotId, healing_);
	return true;
}

void Healing::upgrade() {
	healing_ += std::min(HealingIncrease, std::numeric_limits<int>::max() - healing_);
}
