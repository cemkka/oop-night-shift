#include "EnemyController.h"
#include "abilities/Teleportation.h"

bool EnemyController::teleportTarget(const Field& field, int id, int& targetRow, int& targetColumn) {
	int bestDistance = field.distanceTo(id, field.row(Robot::PlayerId), field.column(Robot::PlayerId));
	if (bestDistance <= Field::NeighborDistance || !field.robot(id).canMove()
		|| !field.visible(id, field.row(Robot::PlayerId), field.column(Robot::PlayerId))) {
		return false;
	}
	bool found = false;
	for (int row = 0; row < field.rows(); ++row) {
		for (int column = 0; column < field.columns(); ++column) {
			if (!field.free(row, column) || field.distanceTo(id, row, column) > Teleportation::Radius) {
				continue;
			}
			int distance = field.distanceTo(Robot::PlayerId, row, column);
			if (distance < bestDistance) {
				bestDistance = distance;
				targetRow = row;
				targetColumn = column;
				found = true;
			}
		}
	}
	return found;
}

void EnemyController::useAbilities(Field& field, int id) {
	const Robot& enemy = field.robot(id);
	if (enemy.side() != Robot::Side::Enemy || !enemy.alive()) {
		return;
	}
	for (int index = 0; index < enemy.abilityCount(); ++index) {
		const Ability& ability = enemy.ability(index);
		while (enemy.alive() && enemy.energy() >= ability.energyCost()) {
			int row = 0;
			int column = 0;
			if (ability.kind() == Ability::Kind::Healing) {
				if (enemy.health() == enemy.maxHealth()) {
					break;
				}
			} else if (ability.kind() != Ability::Kind::Teleportation
				|| !teleportTarget(field, id, row, column)) {
				break;
			}
			if (!field.applyAbility(id, index, row, column)) {
				break;
			}
		}
	}
}
