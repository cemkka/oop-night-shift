#include "abilities/Healing.h"
#include "Field.h"
#include "statuses/Shield.h"
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
	if (!robot.alive() || robot.energy() < energyCost()) {
		return false;
	}
	if (!field.spendEnergy(robotId, energyCost())) {
		return false;
	}
	int excess = std::max(0, healing_ - (robot.maxHealth() - robot.health()));
	field.repair(robotId, healing_);
	if (excess > 0) {
		field.addStatus(robotId, std::make_unique<Shield>(excess));
	}
	return true;
}

void Healing::upgrade() {
	healing_ += std::min(HealingIncrease, std::numeric_limits<int>::max() - healing_);
}
