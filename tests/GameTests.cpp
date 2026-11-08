#include "GameTests.h"
#include "EnemyController.h"
#include "abilities/AbilityCatalog.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include "abilities/AreaAttack.h"
#include "abilities/RangedAttack.h"
#include "statuses/Slowing.h"
#include "statuses/Overload.h"
#include "statuses/Burning.h"
#include "statuses/Shield.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>
#include <utility>

void TestSuite::ownership() {
	static_assert(!std::is_copy_constructible_v<Robot>);
	static_assert(std::is_nothrow_move_constructible_v<Robot>);
	static_assert(std::is_abstract_v<Status>);
	static_assert(std::has_virtual_destructor_v<Status>);
	Robot first(Robot::Side::Friendly);
	Robot second(Robot::Side::Friendly);
	first.addAbility(std::make_unique<Teleportation>());
	second.addAbility(std::make_unique<Teleportation>());
	Field field(12, 12, false);
	int a = field.addRobot(std::move(first), 1, 1);
	int b = field.addRobot(std::move(second), 1, 2);
	field.upgradeAbility(a, 0);
	assert(field.robot(a).ability(0).energyCost() == 4);
	assert(field.robot(b).ability(0).energyCost() == 5);
	for (int row = 2; row < 10; ++row) {
		field.addRobot(Robot(Robot::Side::Enemy), row, 1);
	}
	assert(field.robot(a).ability(0).energyCost() == 4);
	assert(field.robot(b).ability(0).energyCost() == 5);
	assert(!field.applyAbility(a, -1, 1, 3));
	assert(!field.applyAbility(-1, 0, 1, 3));
}

void TestSuite::playerAbilities() {
	for (int choice = 1; choice <= AbilityCatalog::Count; ++choice) {
		Game game(false);
		assert(game.command("bad"));
		assert(game.command("1 trailing"));
		assert(game.command("5"));
		assert(game.field().robot(0).abilityCount() == 0);
		assert(game.command(std::to_string(choice)));
		assert(game.field().robot(0).ability(0).kind() == AbilityCatalog::create(choice)->kind());
	}
	Game healing(false);
	assert(healing.command("1"));
	healing.field_.takeDamage(0, 40);
	healing.field_.spendMovement(0, 3);
	assert(healing.command("r"));
	assert(healing.command("0"));
	assert(healing.command("1"));
	assert(healing.field().robot(0).health() == 80);
	assert(healing.field().robot(0).energy() == 7);
	assert(healing.field().robot(0).remainingMovement() == 0);
	assert(healing.turn() == 1);
	Game teleportation(false);
	assert(teleportation.command("2"));
	assert(teleportation.command("r"));
	assert(teleportation.command("1"));
	assert(teleportation.command("not coordinates"));
	assert(teleportation.command("3 4"));
	assert(teleportation.field().robot(0).energy() == 10);
	for (const auto& target : {"1 6", "1 5", "1 4"}) {
		assert(teleportation.command("r"));
		assert(teleportation.command("1"));
		assert(teleportation.command(target));
	}
	assert(teleportation.field().column(0) == 5);
	assert(teleportation.field().robot(0).energy() == 0);
	assert(teleportation.field().robot(0).remainingMovement() == 3);
	assert(teleportation.turn() == 1);
	Game ranged(false);
	assert(ranged.command("4"));
	assert(ranged.command("r"));
	assert(ranged.command("1"));
	assert(ranged.command("2 1"));
	assert(ranged.field().robot(1).health() == 60);
	assert(ranged.field().robot(0).energy() == 7);
	assert(ranged.turn() == 1);
}

void TestSuite::commandsAndRewards() {
	Game game(false);
	assert(game.command("1"));
	assert(game.command("d"));
	assert(game.field().column(0) == 2);
	assert(game.command("w"));
	assert(game.field().row(0) == 1);
	assert(game.command("s"));
	assert(game.field().row(0) == 2);
	assert(game.command("a"));
	assert(game.field().column(0) == 2);
	assert(game.command("f"));
	assert(game.factory().active());
	assert(game.command("e"));
	assert(game.turn() == 2);
	assert(!game.command("q"));
	Game rewards(false);
	assert(rewards.command("3"));
	assert(rewards.field_.teleport(0, 4, 2));
	for (const auto& position : {std::pair<int, int>{3, 2}, {4, 1}, {4, 3}}) {
		rewards.field_.addRobot(Robot(Robot::Side::Enemy, 1), position.first, position.second);
	}
	assert(rewards.command("r"));
	assert(rewards.command("1"));
	assert(rewards.field().robot(0).rank() == 3);
	assert(rewards.menu_.rewardsPending());
	assert(rewards.command("bad"));
	assert(rewards.menu_.rewardsPending());
	assert(rewards.command("4"));
	assert(rewards.menu_.rewardsPending());
	assert(rewards.command("3"));
	assert(!rewards.menu_.active());
	assert(rewards.field().robot(0).abilityCount() == 2);
	assert(rewards.turn() == 1);
	Game deferred(false);
	assert(deferred.command("2"));
	deferred.field_.takeDamage(2, 40);
	deferred.field_.takeDamage(3, 40);
	deferred.field_.addRobot(Robot(Robot::Side::Enemy, 1), 1, 2);
	deferred.field_.spendMovement(0, 2);
	assert(deferred.command("d"));
	assert(deferred.menu_.rewardsPending());
	assert(deferred.turn() == 1 && deferred.endTurnPending_);
	for (int i = 0; i < 4; ++i) {
		deferred.field_.upgradeAbility(0, 0);
	}
	assert(deferred.command("2"));
	assert(deferred.menu_.rewardsPending());
	assert(deferred.field().robot(0).ability(0).energyCost() == 1);
	assert(deferred.command("1"));
	assert(deferred.turn() == 2);
	assert(!deferred.menu_.active());
	assert(deferred.field_.teleport(0, 5, 6));
	assert(deferred.command("f"));
	assert(!deferred.factory().active());
}

void TestSuite::attacks() {
	Field field(12, 12, false);
	int actor = field.addRobot(Robot(Robot::Side::Friendly), 4, 4);
	int enemy = field.addRobot(Robot(Robot::Side::Enemy, 15), 4, 5);
	int ally = field.addRobot(Robot(Robot::Side::Friendly), 4, 3);
	int diagonal = field.addRobot(Robot(Robot::Side::Enemy, 100), 3, 5);
	AreaAttack area;
	assert(area.apply(field, actor, -999, 999));
	assert(!field.robot(enemy).alive());
	assert(field.cell(4, 5).robotId() == Robot::NoRobotId);
	assert(field.robot(actor).rank() == 2);
	assert(field.robot(actor).health() == field.robot(actor).maxHealth());
	assert(field.robot(ally).health() == 85);
	assert(field.robot(diagonal).health() == 100);
	assert(field.robot(actor).energy() == 8);
	assert(field.robot(actor).remainingMovement() == 3);
	assert(area.apply(field, actor, 0, 0));
	assert(field.robot(ally).health() == 70);
	area.upgrade();
	assert(area.apply(field, actor, 0, 0));
	assert(field.robot(diagonal).health() == 85);
	assert(field.robot(ally).statusCount() == 0);
	assert(!area.apply(field, actor, 0, 0));
	Field rangedField(12, 12, false);
	int caster = rangedField.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	int target = rangedField.addRobot(Robot(Robot::Side::Enemy, 100), 1, 4);
	rangedField.addRobot(Robot(Robot::Side::Enemy, 100), 1, 5);
	rangedField.setTerrain(1, 2, false);
	RangedAttack ranged;
	assert(!ranged.apply(rangedField, caster, 1, 5));
	assert(!ranged.apply(rangedField, caster, 1, 1));
	assert(!ranged.apply(rangedField, caster, -1, 1));
	assert(!ranged.apply(rangedField, caster, 2, 1));
	assert(rangedField.robot(caster).energy() == 10);
	assert(ranged.apply(rangedField, caster, 1, 4));
	assert(rangedField.robot(target).health() == 80);
	ranged.upgrade();
	assert(ranged.apply(rangedField, caster, 1, 4));
	assert(rangedField.robot(target).health() == 50);
	assert(rangedField.robot(target).statusCount() == 0);
	assert(rangedField.robot(caster).remainingMovement() == 3);
	Field empty(8, 8, false);
	empty.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	assert(!area.apply(empty, 0, 1, 1));
	assert(empty.robot(0).energy() == 10);
	int friendly = empty.addRobot(Robot(Robot::Side::Friendly, 1), 1, 2);
	assert(area.apply(empty, 0, 0, 0));
	assert(!empty.robot(friendly).alive());
	assert(empty.robot(0).experience() == 0 && empty.robot(0).rank() == 1);
}

void TestSuite::statuses() {
	Field field(12, 12, false);
	int id = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	field.addAbility(id, std::make_unique<Teleportation>());
	field.beginTurn(id);
	field.addStatus(id, std::make_unique<Slowing>());
	assert(!field.move(id, 1, 2));
	assert(!field.applyAbility(id, 0, 1, 6));
	assert(field.robot(id).energy() == 10);
	field.finishTurn(id);
	assert(!field.robot(id).canMove());
	field.beginTurn(id);
	field.finishTurn(id);
	assert(field.robot(id).canMove());
	field.addStatus(id, std::make_unique<Slowing>());
	field.addStatus(id, std::make_unique<Slowing>());
	assert(field.robot(id).statusCount() == 1);
	int neighbor = field.addRobot(Robot(Robot::Side::Enemy, 100), 2, 1);
	assert(field.interact(id, neighbor));
	assert(field.robot(neighbor).health() == 75);
	field.beginTurn(id);
	field.finishTurn(id);
	assert(!field.robot(id).canMove());
	field.beginTurn(id);
	field.finishTurn(id);
	assert(field.robot(id).canMove());
	field.addStatus(id, std::make_unique<Overload>());
	field.addStatus(id, std::make_unique<Overload>());
	assert(field.robot(id).statusCount() == 1);
	assert(!field.applyAbility(id, 0, 1, 6));
	assert(field.robot(id).energy() == 10);
	assert(field.robot(id).statusCount() == 0);
	assert(field.applyAbility(id, 0, 1, 6));
	assert(field.robot(id).energy() == 5);
	field.addStatus(id, std::make_unique<Shield>(10));
	field.takeDamage(id, 5);
	field.takeDamage(id, 10);
	assert(field.robot(id).health() == 100);
	assert(field.robot(id).statusCount() == 1);
	field.addStatus(id, std::make_unique<Shield>(5));
	field.takeDamage(id, 16);
	assert(field.robot(id).health() == 99);
	assert(field.robot(id).statusCount() == 0);
	field.addStatus(id, std::make_unique<Burning>(1));
	field.addStatus(id, std::make_unique<Burning>(2));
	for (int tick = 0; tick < 3; ++tick) {
		field.finishTurn(id);
		assert(field.robot(id).health() == 99 - (tick + 1) * Burning::Damage);
	}
	assert(field.robot(id).statusCount() == 0);
	field.addStatus(id, std::make_unique<Burning>(2));
	field.addStatus(id, std::make_unique<Shield>(Burning::Damage));
	field.finishTurn(id);
	assert(field.robot(id).health() == 84);
	field.finishTurn(id);
	assert(field.robot(id).health() == 84);
	assert(field.robot(id).statusCount() == 1);
	int attacker = field.addRobot(Robot(Robot::Side::Enemy), 1, 5);
	field.addStatus(id, std::make_unique<Shield>(5));
	assert(field.interact(attacker, id));
	assert(field.robot(id).health() == 69);
	assert(field.robot(id).statusCount() == 0);
	field.addStatus(id, std::make_unique<Shield>(10));
	field.addAbility(attacker, std::make_unique<RangedAttack>());
	assert(field.applyAbility(attacker, 1, 1, 6));
	assert(field.robot(id).health() == 59);
	Robot saturated(Robot::Side::Friendly);
	saturated.addStatus(std::make_unique<Shield>(std::numeric_limits<int>::max()));
	saturated.addStatus(std::make_unique<Shield>(1));
	saturated.takeDamage(std::numeric_limits<int>::max());
	assert(saturated.health() == 100);
	saturated.addStatus(std::make_unique<Slowing>(std::numeric_limits<int>::max()));
	saturated.addStatus(std::make_unique<Slowing>());
	assert(!saturated.canMove());
	saturated.addStatus(std::make_unique<Burning>(std::numeric_limits<int>::max()));
	saturated.addStatus(std::make_unique<Burning>(1));
	saturated.finishTurn();
	assert(saturated.health() == 100);
	for (int invalid = 0; invalid < 3; ++invalid) {
		bool rejected = false;
		try {
			if (invalid == 0) { Burning status(0); }
			if (invalid == 1) { Shield status(0); }
			if (invalid == 2) { Slowing status(0); }
		} catch (const std::invalid_argument&) {
			rejected = true;
		}
		assert(rejected);
	}
}

void TestSuite::cells() {
	Field field(12, 12, false);
	int a = field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	int b = field.addRobot(Robot(Robot::Side::Friendly), 2, 2);
	field.addCellStatus(1, 2, std::make_unique<Shield>(10));
	field.addCellStatus(1, 2, std::make_unique<Burning>(2));
	assert(field.move(a, 1, 2));
	assert(field.robot(a).statusCount() == 2);
	assert(field.cell(1, 2).statusCount() == 1);
	assert(field.teleport(a, 1, 3));
	assert(field.move(b, 1, 2));
	assert(field.cell(1, 2).statusCount() == 0);
	assert(field.robot(b).statusCount() == 1);
	field.takeDamage(a, 11);
	assert(field.robot(b).statusCount() == 1);
	assert(field.robot(b).health() == 100);
	std::string independent = field.robot(b).status(0).name();
	field.finishTurn(a);
	assert(field.robot(b).status(0).name() == independent);
	field.addCellStatus(1, 4, std::make_unique<Slowing>());
	Teleportation teleportation;
	assert(teleportation.apply(field, a, 1, 4));
	assert(!field.robot(a).canMove());
	assert(field.cell(1, 4).statusCount() == 0);
	assert(field.row(a) == 1 && field.column(a) == 4);
	assert(field.cell(1, 3).robotId() == Robot::NoRobotId);
	assert(field.cell(1, 4).robotId() == a);
	bool rejected = false;
	try {
		field.addCellStatus(0, 0, std::make_unique<Overload>());
	} catch (const std::invalid_argument&) { rejected = true; }
	assert(rejected);
	field.addCellStatus(4, 4, std::make_unique<Shield>(10));
	field.addCellStatus(5, 5, std::make_unique<Shield>(1));
	field.addCellStatus(5, 5, std::make_unique<Overload>());
	rejected = false;
	try {
		field.addCellStatus(5, 5, std::make_unique<Slowing>());
	} catch (const std::invalid_argument&) { rejected = true; }
	assert(rejected);
	field.setTerrain(4, 4, false);
	assert(field.cell(4, 4).statusCount() == 0);
	int counts[3] = {0, 0, 0};
	for (int sample = 0; sample < 12; ++sample) {
		Field random(30, 30);
		for (int row = 0; row < random.rows(); ++row) {
			for (int column = 0; column < random.columns(); ++column) {
				int count = random.cell(row, column).statusCount();
				assert(count >= 0 && count <= 2);
				if (!random.cell(row, column).passable()) {
					assert(count == 0);
				} else {
					++counts[count];
				}
			}
		}
	}
	assert(counts[0] > counts[1] && counts[1] > counts[2]);
	Game game;
	for (int row = 0; row < game.field().rows(); ++row) {
		for (int column = 0; column < game.field().columns(); ++column) {
			if (!game.field().cell(row, column).passable()) {
				assert(game.field().cell(row, column).statusCount() == 0);
			}
		}
	}
}

void TestSuite::enemies() {
	Field field(30, 30, false);
	field.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	Robot healer(Robot::Side::Enemy, 100);
	if (healer.ability(0).kind() != Ability::Kind::Healing) {
		healer.addAbility(std::make_unique<Healing>());
	}
	int healing = field.addRobot(std::move(healer), 20, 20);
	field.takeDamage(healing, 60);
	EnemyController::useAbilities(field, healing);
	assert(field.robot(healing).health() == 100);
	assert(field.robot(healing).energy() == 1);
	Robot teleporter(Robot::Side::Enemy, 100, 5, 1, 30);
	if (teleporter.ability(0).kind() != Ability::Kind::Teleportation) {
		teleporter.addAbility(std::make_unique<Teleportation>());
	}
	int teleport = field.addRobot(std::move(teleporter), 1, 20);
	int index = AbilityCatalog::find(field.robot(teleport), Ability::Kind::Teleportation);
	for (int i = 0; i < 4; ++i) {
		field.upgradeAbility(teleport, index);
	}
	EnemyController::useAbilities(field, teleport);
	assert(field.distanceTo(teleport, 1, 1) == 1);
	assert(field.robot(teleport).energy() == 6);
	assert(field.robot(teleport).remainingMovement() == 1);
	EnemyController::useAbilities(field, teleport);
	assert(field.robot(teleport).energy() == 6);
	Game game(false);
	for (int id = 2; id < game.field().robotCount(); ++id) {
		const Robot& robot = game.field().robot(id);
		assert(robot.abilityCount() == 1);
		assert(robot.ability(0).kind() == Ability::Kind::Healing
			|| robot.ability(0).kind() == Ability::Kind::Teleportation);
	}
	int count = game.field().robotCount();
	for (int tick = 0; tick < 5; ++tick) {
		game.factory_.advance();
	}
	assert(game.field().robotCount() == count + 1);
	assert(game.field().robot(count).abilityCount() == 1);
	assert(game.field().robot(count).ability(0).kind() == Ability::Kind::Healing
		|| game.field().robot(count).ability(0).kind() == Ability::Kind::Teleportation);
	game.computerTurn();
}

std::string TestSuite::runGame(Game& game, const std::string& input) {
	std::istringstream source(input);
	std::ostringstream output;
	auto* previousInput = std::cin.rdbuf(source.rdbuf());
	auto* previousOutput = std::cout.rdbuf(output.rdbuf());
	std::cin.clear();
	game.run();
	std::cin.rdbuf(previousInput);
	std::cout.rdbuf(previousOutput);
	std::cin.clear();
	return output.str();
}

void TestSuite::streams() {
	for (const auto& input : {"", "1\nr\n", "2\nr\n1\n", "4\nr\n1\n999999999999999999999\n"}) {
		Game game(false);
		runGame(game, input);
		assert(game.field().robot(0).energy() == 10);
	}
	for (const auto& input : {"Q\n", "2\nr\nq\n", "4\nr\n1\nQ\n"}) {
		Game game(false);
		runGame(game, input);
		assert(game.field().robot(0).energy() == 10);
	}
	Game output(false);
	std::string text = runGame(output, "invalid\n2\nr\n1\n1 6\nq\n");
	assert(text.find("нумерация строк и столбцов с 0") != std::string::npos);
	assert(text.find("Телепортация — 5 энергии") != std::string::npos);
	assert(text.find("4 — Дальний удар") != std::string::npos);
	assert(output.field().column(0) == 6);
	assert(output.field().robot(0).energy() == 5);
	Game rewardEof(false);
	assert(rewardEof.command("1"));
	rewardEof.field_.addRobot(Robot(Robot::Side::Enemy, 1), 1, 2);
	assert(rewardEof.command("d"));
	runGame(rewardEof, "");
	assert(rewardEof.menu_.rewardsPending());
	assert(!rewardEof.command("Q"));
}

void TestSuite::endings() {
	Game lost(false);
	assert(lost.command("1"));
	lost.field_.takeDamage(2, 40);
	lost.field_.takeDamage(3, 40);
	lost.field_.takeDamage(1, 80);
	lost.field_.takeDamage(0, 96);
	lost.field_.addStatus(0, std::make_unique<Burning>(1));
	assert(lost.command("e"));
	assert(lost.lost());
	assert(!lost.won());
	assert(lost.field().cell(1, 1).robotId() == Robot::NoRobotId);
	assert(runGame(lost, "").find("Поражение") != std::string::npos);
	Game won(false);
	assert(won.command("1"));
	won.field_.takeDamage(2, 40);
	won.field_.takeDamage(3, 40);
	assert(won.field_.teleport(0, 5, 6));
	assert(won.command("f"));
	assert(won.won());
	assert(runGame(won, "").find("Победа") != std::string::npos);
	Game burnVictory(false);
	assert(burnVictory.command("1"));
	burnVictory.field_.takeDamage(2, 39);
	burnVictory.field_.spendEnergy(2, 10);
	burnVictory.field_.takeDamage(3, 40);
	burnVictory.field_.addStatus(2, std::make_unique<Burning>(1));
	assert(burnVictory.field_.teleport(0, 5, 6));
	assert(burnVictory.command("f"));
	assert(burnVictory.command("e"));
	assert(burnVictory.won());
	assert(burnVictory.field().cell(2, 6).robotId() == Robot::NoRobotId);
	Game attackVictory(false);
	assert(attackVictory.command("3"));
	attackVictory.field_.takeDamage(2, 40);
	attackVictory.field_.takeDamage(3, 40);
	assert(attackVictory.field_.teleport(0, 5, 6));
	assert(attackVictory.command("f"));
	attackVictory.field_.addRobot(Robot(Robot::Side::Enemy, 1), 5, 5);
	assert(attackVictory.command("r"));
	assert(attackVictory.command("1"));
	assert(attackVictory.won());
	assert(attackVictory.menu_.rewardsPending());
	assert(attackVictory.command("4"));
	assert(!attackVictory.menu_.active());
	assert(runGame(attackVictory, "").find("Победа") != std::string::npos);
}

void TestSuite::run() {
	ownership();
	playerAbilities();
	commandsAndRewards();
	attacks();
	statuses();
	cells();
	enemies();
	streams();
	endings();
	std::cout << "All mandatory lab2 tests passed\n";
}

int main() {
	TestSuite::run();
}
