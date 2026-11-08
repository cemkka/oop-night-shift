#include "AbilitiesTests.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include "abilities/AreaAttack.h"
#include "abilities/RangedAttack.h"
#include "statuses/Overload.h"
#include "statuses/Burning.h"
#include "statuses/Shield.h"
#include "statuses/ItemGrant.h"
#include "Field.h"
#include <cassert>
#include <algorithm>
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
		result.push_back(robot.statusCount());
		for (int index = 0; index < robot.statusCount(); ++index) {
			std::string name = robot.status(index).name();
			result.push_back(static_cast<int>(name.size()));
			for (unsigned char character : name) {
				result.push_back(character);
			}
		}
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
	assert(field.robot(id).statusCount() == 0);
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

void AbilityTests::testAreaStatuses() {
	Field field(12, 12, false);
	int actor = field.addRobot(Robot(Robot::Side::Friendly), 4, 4);
	int ally = field.addRobot(Robot(Robot::Side::Friendly), 4, 5);
	int enemy = field.addRobot(Robot(Robot::Side::Enemy, 100), 3, 4);
	int doomed = field.addRobot(Robot(Robot::Side::Enemy, 15), 4, 3);
	int outside = field.addRobot(Robot(Robot::Side::Enemy, 100), 5, 5);
	int shielded = field.addRobot(Robot(Robot::Side::Enemy, 100), 5, 4);
	field.addStatus(shielded, std::make_unique<Shield>(20));
	AreaAttack area;
	assert(area.apply(field, actor, -1, -1));
	assert(field.robot(actor).rank() == 2);
	assert(field.robot(actor).health() == field.robot(actor).maxHealth());
	assert(field.robot(actor).statusCount() == 0);
	assert(field.robot(actor).remainingMovement() == 3);
	assert(field.robot(actor).energy() == 8);
	assert(!field.robot(doomed).alive() && field.robot(doomed).statusCount() == 0);
	assert(field.robot(outside).health() == 100 && field.robot(outside).statusCount() == 0);
	for (int id : {ally, enemy}) {
		assert(field.robot(id).health() == 85);
		assert(field.robot(id).statusCount() == 1);
		assert(dynamic_cast<const Overload*>(&field.robot(id).status(0)));
	}
	assert(field.robot(shielded).health() == 100);
	assert(field.robot(shielded).statusCount() == 2);
	assert(dynamic_cast<const Overload*>(&field.robot(shielded).status(1)));
	assert(area.apply(field, actor, 0, 0));
	for (int id : {ally, enemy}) {
		assert(field.robot(id).statusCount() == 1);
		assert(field.robot(id).health() == 70);
	}
	assert(field.robot(shielded).statusCount() == 2);
	Healing healing;
	assert(!healing.apply(field, ally, 0, 0));
	assert(field.robot(ally).energy() == 10 && field.robot(ally).health() == 70);
	assert(field.robot(ally).statusCount() == 0);
	assert(healing.apply(field, ally, 0, 0));
	assert(field.robot(ally).health() == 90 && field.robot(ally).energy() == 7);
	assert(field.spendEnergy(actor, 1));
	expectFailure(area, field, actor, 0, 0);
	auto before = snapshot(field);
	field.addStatus(actor, std::make_unique<Overload>());
	assert(!area.apply(field, actor, 0, 0));
	assert(snapshot(field) == before);
}

void AbilityTests::testRangedStatuses() {
	Field field(12, 12, false);
	int actor = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	int target = field.addRobot(Robot(Robot::Side::Enemy, 200), 1, 4);
	field.addStatus(target, std::make_unique<Burning>(1));
	RangedAttack ranged;
	assert(ranged.apply(field, actor, 1, 4));
	assert(field.robot(target).health() == 180);
	assert(field.robot(target).statusCount() == 1);
	assert(dynamic_cast<const Burning*>(&field.robot(target).status(0)));
	field.finishTurn(target);
	assert(field.robot(target).health() == 180 - Burning::Damage);
	assert(ranged.apply(field, actor, 1, 4));
	assert(field.robot(target).health() == 155);
	for (int tick = 0; tick < 6; ++tick) {
		field.finishTurn(target);
		assert(field.robot(target).health() == 155 - (tick + 1) * Burning::Damage);
	}
	assert(field.robot(target).statusCount() == 0);
	assert(field.robot(actor).energy() == 4);
	assert(field.robot(actor).remainingMovement() == 3);
	assert(field.spendEnergy(actor, 2));
	expectFailure(ranged, field, actor, 1, 4);
	auto before = snapshot(field);
	field.addStatus(actor, std::make_unique<Overload>());
	assert(!ranged.apply(field, actor, -1, -1));
	assert(snapshot(field) == before);
	Field lethal(8, 8, false);
	lethal.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	lethal.addRobot(Robot(Robot::Side::Enemy, 20), 1, 2);
	assert(ranged.apply(lethal, 0, 1, 2));
	assert(!lethal.robot(1).alive() && lethal.robot(1).statusCount() == 0);
	assert(lethal.robot(0).rank() == 2);
	Field shielded(8, 8, false);
	shielded.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	shielded.addRobot(Robot(Robot::Side::Enemy, 100), 1, 2);
	shielded.addStatus(1, std::make_unique<Shield>(20));
	assert(ranged.apply(shielded, 0, 1, 2));
	assert(shielded.robot(1).health() == 100 && shielded.robot(1).statusCount() == 2);
	assert(dynamic_cast<const Burning*>(&shielded.robot(1).status(1)));
}

void AbilityTests::testHealingShield() {
	Healing healing;
	for (int missing : {40, 20, 5, 0}) {
		Field field(8, 8, false);
		field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
		field.takeDamage(0, missing);
		field.spendMovement(0, 3);
		assert(healing.apply(field, 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
		assert(field.robot(0).health() == std::min(100, 120 - missing));
		assert(field.robot(0).energy() == 7 && field.robot(0).remainingMovement() == 0);
		int excess = std::max(0, 20 - missing);
		assert(field.robot(0).statusCount() == (excess > 0 ? 1 : 0));
		if (excess > 0) {
			assert(dynamic_cast<const Shield*>(&field.robot(0).status(0)));
			field.takeDamage(0, excess);
			assert(field.robot(0).health() == 100);
			assert(field.robot(0).statusCount() == 1);
			field.takeDamage(0, excess + 1);
			assert(field.robot(0).health() == 99 && field.robot(0).statusCount() == 0);
		}
	}
	Field merged(8, 8, false);
	merged.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	merged.addStatus(0, std::make_unique<Shield>(7));
	assert(healing.apply(merged, 0, 0, 0));
	assert(healing.apply(merged, 0, 0, 0));
	assert(merged.robot(0).statusCount() == 1);
	merged.takeDamage(0, 47);
	assert(merged.robot(0).health() == 100);
	merged.takeDamage(0, 48);
	assert(merged.robot(0).health() == 99);
	assert(merged.robot(0).statusCount() == 0);
	Healing improved;
	improved.upgrade();
	assert(improved.apply(merged, 0, 0, 0));
	assert(merged.robot(0).health() == 100 && merged.robot(0).energy() == 1);
	merged.takeDamage(0, 29);
	assert(merged.robot(0).health() == 100);
	expectFailure(healing, merged, 0, 0, 0);
	auto before = snapshot(merged);
	merged.addStatus(0, std::make_unique<Overload>());
	assert(!healing.apply(merged, 0, 0, 0));
	assert(snapshot(merged) == before);
	Field capped(8, 8, false);
	capped.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	capped.addStatus(0, std::make_unique<Shield>(std::numeric_limits<int>::max()));
	assert(healing.apply(capped, 0, 0, 0));
	capped.takeDamage(0, std::numeric_limits<int>::max());
	assert(capped.robot(0).health() == 100);
}

void AbilityTests::testRepairWithoutShield() {
	Field field(8, 8, false);
	field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	field.addRobot(Robot(Robot::Side::Friendly), 1, 2);
	field.takeDamage(1, 5);
	assert(field.interact(0, 1));
	assert(field.robot(1).health() == 100 && field.robot(1).statusCount() == 0);
	field.addStatus(0, std::make_unique<ItemGrant>(Inventory::Item::Health));
	field.takeDamage(0, 5);
	assert(field.useItem(0, Inventory::Item::Health));
	assert(field.robot(0).health() == 100 && field.robot(0).statusCount() == 0);
	field.takeDamage(0, 1);
	assert(field.robot(0).health() == 99);
}

void AbilityTests::run() {
	testHealing();
	testTeleportation();
	testDeadRobotsAndEnergy();
	testMovementPreserved();
	testAreaStatuses();
	testRangedStatuses();
	testHealingShield();
	testRepairWithoutShield();
	std::cout << "All ability tests passed\n";
}

int main() {
	AbilityTests::run();
}
