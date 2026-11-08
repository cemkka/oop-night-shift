#include "abilities/RangedAttack.h"
#include "Field.h"
#include "statuses/Burning.h"
#include <algorithm>
#include <limits>

Ability::Kind RangedAttack::kind() const { return Kind::RangedAttack; }

std::string RangedAttack::name() const { return "Дальний удар"; }

int RangedAttack::energyCost() const { return EnergyCost; }

bool RangedAttack::canUpgrade() const { return damage_ < std::numeric_limits<int>::max(); }

bool RangedAttack::apply(Field& field, int robotId, int targetRow, int targetColumn) {
	if (!field.prepareAbility(robotId) || !field.contains(targetRow, targetColumn)
		|| field.distanceTo(robotId, targetRow, targetColumn) > Radius) {
		return false;
	}
	int target = field.cell(targetRow, targetColumn).robotId();
	if (target == Robot::NoRobotId || target == robotId || !field.robot(target).alive()
		|| !field.spendEnergy(robotId, energyCost())) {
		return false;
	}
	field.damageFrom(robotId, target, damage_);
	if (field.robot(target).alive()) {
		field.addStatus(target, std::make_unique<Burning>(BurningDuration, robotId));
	}
	return true;
}

void RangedAttack::upgrade() {
	damage_ += std::min(DamageIncrease, std::numeric_limits<int>::max() - damage_);
}
