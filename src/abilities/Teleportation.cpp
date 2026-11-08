#include "abilities/Teleportation.h"
#include "Field.h"
#include <algorithm>

std::string Teleportation::name() const { return "Телепортация"; }

int Teleportation::energyCost() const { return energyCost_; }

bool Teleportation::apply(Field& field, int robotId, int targetRow, int targetColumn) {
	if (robotId < 0 || robotId >= field.robotCount()) {
		return false;
	}
	const Robot& robot = field.robot(robotId);
	// free() also rejects the robot's current cell and out-of-bounds targets.
	if (!robot.alive() || !field.free(targetRow, targetColumn)
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
