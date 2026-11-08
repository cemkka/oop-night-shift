#include "abilities/Teleportation.h"
#include "Field.h"
#include <algorithm>

std::string Teleportation::name() const { return "Телепортация"; }

Ability::Kind Teleportation::kind() const { return Kind::Teleportation; }

int Teleportation::energyCost() const { return energyCost_; }

bool Teleportation::canUpgrade() const { return energyCost_ > MinimumEnergyCost; }

bool Teleportation::apply(Field& field, int robotId, int targetRow, int targetColumn) {
	if (!field.prepareAbility(robotId)) {
		return false;
	}
	const Robot& robot = field.robot(robotId);
	if (!robot.canMove() || !field.free(targetRow, targetColumn)
		|| field.distanceTo(robotId, targetRow, targetColumn) > Radius
		|| robot.energy() < energyCost()) {
		return false;
	}
	if (!field.spendEnergy(robotId, energyCost())) {
		return false;
	}
	return field.teleport(robotId, targetRow, targetColumn);
}

void Teleportation::upgrade() {
	energyCost_ -= std::min(EnergyCostDecrease, energyCost_ - MinimumEnergyCost);
}
