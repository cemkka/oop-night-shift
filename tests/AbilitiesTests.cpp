#include "AbilitiesTests.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include "Field.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

static_assert(std::is_abstract_v<Ability>);
static_assert(std::has_virtual_destructor_v<Ability>);
static_assert(!std::is_abstract_v<Healing>);
static_assert(!std::is_abstract_v<Teleportation>);

std::vector<int> AbilityTests::snapshot(const Field& field) {
	std::vector<int> result;
	for (int id = 0; id < field.robotCount(); ++id) {
		const Robot& robot = field.robot(id);
		result.insert(result.end(), {robot.health(), robot.maxHealth(), robot.energy(),
			robot.maxEnergy(), robot.attack(), robot.experience(), robot.experienceToNextRank(),
			robot.rank(), robot.speed(), robot.vision(), robot.remainingMovement(),
			static_cast<int>(robot.side()), field.row(id), field.column(id)});
	}
	for (int row = 0; row < field.rows(); ++row) {
		for (int column = 0; column < field.columns(); ++column) {
			const Cell& cell = field.cell(row, column);
			result.insert(result.end(), {cell.robotId(), cell.known(), cell.passable(), cell.cost()});
		}
	}
	return result;
}

void AbilityTests::expectFailure(Ability& ability, Field& field, int id, int row, int column) {
	const auto before = snapshot(field);
	assert(!ability.apply(field, id, row, column));
	assert(before == snapshot(field));
}

void AbilityTests::testHealing() {
	Field field(12, 12, false);
	int id = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	int other = field.addRobot(Robot(Robot::Side::Friendly), 1, 2);
	field.takeDamage(id, 50);
	field.takeDamage(other, 50);
	assert(field.spendMovement(id, field.robot(id).remainingMovement()));
	Healing healing;
	int energy = field.robot(id).energy();
	assert(healing.apply(field, id, 1, 2));
	assert(field.robot(id).health() == 70);
	assert(field.robot(other).health() == 50);
	assert(field.robot(id).energy() == energy - healing.energyCost());
	assert(field.robot(id).remainingMovement() == 0);
	healing.upgrade();
	assert(healing.apply(field, id, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
	assert(field.robot(id).health() == field.robot(id).maxHealth());
	expectFailure(healing, field, id, 0, 0);
	field.takeDamage(id, 5);
	assert(healing.apply(field, id, -1, -1));
	assert(field.robot(id).health() == field.robot(id).maxHealth());
	field.takeDamage(id, 10);
	expectFailure(healing, field, id, 1, 1);
	expectFailure(healing, field, -1, 1, 1);
	expectFailure(healing, field, field.robotCount(), 1, 1);
}

void AbilityTests::testTeleportation() {
	Field field(12, 12, false);
	int id = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	field.addRobot(Robot(Robot::Side::Enemy), 2, 1);
	// A complete wall blocks every ordinary path to the destination.
	for (int row = 1; row < field.rows() - 1; ++row) {
		field.setTerrain(row, 3, false);
	}
	field.setTerrain(1, 6, true, Cell::SlowMovementCost);
	field.remember(id);
	assert(!field.cell(1, 10).known());
	Teleportation teleportation;
	expectFailure(teleportation, field, id, 1, 7);
	expectFailure(teleportation, field, id, 1, 3);
	expectFailure(teleportation, field, id, 2, 1);
	expectFailure(teleportation, field, id, 1, 1);
	expectFailure(teleportation, field, id, -1, 1);
	expectFailure(teleportation, field, id, 12, 1);
	expectFailure(teleportation, field, id, 1, -1);
	expectFailure(teleportation, field, id, 1, 12);
	expectFailure(teleportation, field, id, std::numeric_limits<int>::max(), 1);
	expectFailure(teleportation, field, -1, 1, 6);
	expectFailure(teleportation, field, field.robotCount(), 1, 6);
	assert(field.spendMovement(id, field.robot(id).remainingMovement()));
	int energy = field.robot(id).energy();
	assert(teleportation.apply(field, id, 1, 6));
	assert(field.robot(id).energy() == energy - teleportation.energyCost());
	assert(field.robot(id).remainingMovement() == 0);
	assert(field.row(id) == 1 && field.column(id) == 6);
	assert(field.cell(1, 1).robotId() == Robot::NoRobotId);
	assert(field.cell(1, 6).robotId() == id);
	assert(field.cell(1, 10).known());
	// Diagonal displacement (2 + 3) has Manhattan distance 5.
	teleportation.upgrade();
	assert(teleportation.energyCost() == 4);
	assert(teleportation.apply(field, id, 3, 9));
	assert(field.robot(id).energy() == 1);
	assert(field.cell(1, 6).robotId() == Robot::NoRobotId);
	assert(field.cell(3, 9).robotId() == id);
	assert(field.row(id) == 3 && field.column(id) == 9);
	expectFailure(teleportation, field, id, 3, 8);
	for (int i = 0; i < 1000; ++i) {
		teleportation.upgrade();
	}
	assert(teleportation.energyCost() == 1);
	assert(teleportation.apply(field, id, 3, 8));
	assert(field.robot(id).energy() == 0);
	expectFailure(teleportation, field, id, 3, 9);
}

void AbilityTests::testDeadRobotsAndEnergy() {
	Field field(12, 12, false);
	int id = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	Healing healing;
	Teleportation teleportation;
	assert(!field.spendEnergy(id, 0));
	assert(!field.spendEnergy(id, -1));
	assert(!field.spendEnergy(id, 11));
	assert(field.robot(id).energy() == 10);
	assert(field.spendEnergy(id, 8));
	field.takeDamage(id, 20);
	expectFailure(healing, field, id, 0, 0);
	expectFailure(teleportation, field, id, 1, 6);
	field.finishTurn(id);
	field.takeDamage(id, field.robot(id).health());
	expectFailure(healing, field, id, 0, 0);
	expectFailure(teleportation, field, id, 1, 6);
	const auto before = snapshot(field);
	assert(!field.teleport(id, 1, 6));
	assert(!field.spendEnergy(id, 1));
	assert(before == snapshot(field));
	std::unique_ptr<Ability> ability = std::make_unique<Healing>();
	assert(!ability->name().empty());
	ability = std::make_unique<Teleportation>();
	assert(!ability->name().empty());
}

void AbilityTests::testMovementPreserved() {
	Field field(12, 12, false);
	int id = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	field.takeDamage(id, 30);
	int movement = field.robot(id).remainingMovement();
	Healing healing;
	Teleportation teleportation;
	assert(healing.apply(field, id, 0, 0));
	assert(field.robot(id).remainingMovement() == movement);
	assert(teleportation.apply(field, id, 1, 6));
	assert(field.robot(id).remainingMovement() == movement);
}

void AbilityTests::run() {
	testHealing();
	testTeleportation();
	testDeadRobotsAndEnergy();
	testMovementPreserved();
	std::cout << "All ability tests passed\n";
}

int main() {
	AbilityTests::run();
}
