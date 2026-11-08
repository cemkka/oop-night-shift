#include "GameTests.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <utility>

void TestSuite::ownership() {
	static_assert(!std::is_copy_constructible_v<Robot>);
	static_assert(std::is_nothrow_move_constructible_v<Robot>);
	Robot first(Robot::Side::Friendly);
	Robot second(Robot::Side::Friendly);
	first.addAbility(std::make_unique<Teleportation>());
	second.addAbility(std::make_unique<Teleportation>());
	Field field(12, 12);
	int firstId = field.addRobot(std::move(first), 1, 1);
	int secondId = field.addRobot(std::move(second), 1, 2);
	field.upgradeAbility(firstId, 0);
	assert(field.robot(firstId).ability(0).energyCost() == 4);
	assert(field.robot(secondId).ability(0).energyCost() == 5);
	for (int row = 2; row < 10; ++row) {
		field.addRobot(Robot(Robot::Side::Enemy), row, 1);
	}
	assert(field.robot(firstId).ability(0).energyCost() == 4);
	assert(field.robot(secondId).ability(0).energyCost() == 5);
	assert(!field.applyAbility(firstId, -1, 1, 3));
	assert(!field.applyAbility(-1, 0, 1, 3));
}

void TestSuite::playerAbilities() {
	Game healing;
	assert(healing.command("bad"));
	assert(healing.command("1 trailing"));
	assert(healing.field().robot(0).abilityCount() == 0);
	assert(healing.command("1"));
	assert(dynamic_cast<const Healing*>(&healing.field().robot(0).ability(0)));
	healing.field_.takeDamage(0, 40);
	healing.field_.spendMovement(0, healing.field().robot(0).remainingMovement());
	assert(healing.command("r"));
	assert(healing.command("0"));
	assert(healing.inputMode_ == Game::InputMode::Ability);
	assert(healing.command("1"));
	assert(healing.field().robot(0).health() == 80);
	assert(healing.field().robot(0).energy() == 7);
	assert(healing.field().robot(0).remainingMovement() == 0);
	assert(healing.turn() == 1);
	Game teleportation;
	assert(teleportation.command("2"));
	assert(teleportation.command("r"));
	assert(teleportation.command("1"));
	assert(teleportation.command("not coordinates"));
	assert(teleportation.inputMode_ == Game::InputMode::Target);
	assert(teleportation.command("3 4"));
	assert(teleportation.field().robot(0).energy() == 10);
	assert(teleportation.field().row(0) == 1 && teleportation.field().column(0) == 1);
	assert(teleportation.command("r"));
	assert(teleportation.command("1"));
	assert(teleportation.command("1 6"));
	assert(teleportation.field().column(0) == 6);
	assert(teleportation.field().robot(0).energy() == 5);
	assert(teleportation.field().robot(0).remainingMovement() == 3);
	assert(teleportation.turn() == 1);
	assert(teleportation.command("r"));
	assert(teleportation.command("1"));
	assert(teleportation.command("1 5"));
	assert(teleportation.field().robot(0).energy() == 0);
	assert(teleportation.command("r"));
	assert(teleportation.command("1"));
	assert(teleportation.command("1 4"));
	assert(teleportation.field().column(0) == 5);
	assert(teleportation.field().cell(1, 5).robotId() == 0);
	assert(teleportation.field().robot(0).energy() == 0);
	assert(teleportation.turn() == 1);
}

void TestSuite::commandsAndRewards() {
	Game game;
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
	Game rewards;
	assert(rewards.command("2"));
	rewards.field_.takeDamage(2, rewards.field().robot(2).health());
	rewards.field_.takeDamage(3, rewards.field().robot(3).health());
	for (int kill = 0; kill < 3; ++kill) {
		rewards.field_.beginTurn(0);
		rewards.field_.finishTurn(0);
		rewards.field_.spendMovement(0, 2);
		rewards.field_.addRobot(Robot(Robot::Side::Enemy, 1), 1, 2);
		int turn = rewards.turn();
		assert(rewards.command("d"));
		if (rewards.inputMode_ == Game::InputMode::RankReward) {
			assert(rewards.turn() == turn);
			assert(rewards.endTurnPending_);
			int rewarded = rewards.rewardedRank_;
			assert(rewards.command("bad"));
			assert(rewards.rewardedRank_ == rewarded);
			assert(rewards.command("1"));
			assert(rewards.turn() == turn + 1);
		}
	}
	assert(rewards.field().robot(0).rank() == 3);
	assert(rewards.field().robot(0).abilityCount() == 2);
	rewards.rewardedRank_ = 1;
	assert(rewards.command("?"));
	assert(rewards.inputMode_ == Game::InputMode::RankReward);
	assert(rewards.command("2"));
	assert(rewards.inputMode_ == Game::InputMode::RankReward);
	assert(rewards.command("2"));
	assert(rewards.inputMode_ == Game::InputMode::Command);
	int index = rewards.findAbility(true);
	while (rewards.field().robot(0).ability(index).energyCost() > 1) {
		rewards.field_.upgradeAbility(0, index);
	}
	--rewards.rewardedRank_;
	assert(rewards.command("?"));
	int rewarded = rewards.rewardedRank_;
	assert(rewards.command("2"));
	assert(rewards.rewardedRank_ == rewarded);
	assert(rewards.inputMode_ == Game::InputMode::RankReward);
	assert(rewards.command("1"));
	assert(rewards.inputMode_ == Game::InputMode::Command);
	assert(rewards.field().robot(0).ability(index).energyCost() == 1);
	assert(rewards.field_.teleport(0, 5, 6));
	assert(rewards.command("f"));
	assert(!rewards.factory().active());
}

void TestSuite::enemies() {
	bool sawHealing = false;
	bool sawTeleportation = false;
	for (int attempt = 0; attempt < 64; ++attempt) {
		Game game;
		for (int id = 2; id < game.field().robotCount(); ++id) {
			assert(game.field().robot(id).abilityCount() == 1);
			if (dynamic_cast<const Healing*>(&game.field().robot(id).ability(0))) {
				sawHealing = true;
				game.field_.takeDamage(id, 25);
				game.enemyAbility(id);
				assert(game.field().robot(id).health() == 35);
				assert(game.field().robot(id).energy() == 7);
			} else {
				sawTeleportation = true;
				assert(dynamic_cast<const Teleportation*>(&game.field().robot(id).ability(0)));
				for (int upgrade = 0; upgrade < 4; ++upgrade) {
					game.field_.upgradeAbility(id, 0);
				}
				int distance = game.field().distanceTo(id, 1, 1);
				game.enemyAbility(id);
				assert(game.field().distanceTo(id, 1, 1) < distance);
				assert(game.field().robot(id).energy() == 9);
				assert(game.field().robot(id).remainingMovement() == 1);
			}
		}
		int count = game.field().robotCount();
		for (int turn = 0; turn < 5; ++turn) {
			game.factory_.advance();
		}
		assert(game.field().robotCount() == count + 1);
		const Robot& spawned = game.field().robot(count);
		assert(spawned.abilityCount() == 1);
		assert(dynamic_cast<const Healing*>(&spawned.ability(0))
			|| dynamic_cast<const Teleportation*>(&spawned.ability(0)));
		game.computerTurn();
		for (int id = 2; id < game.field().robotCount(); ++id) {
			assert(game.field().robot(id).energy() >= 0);
		}
		Game singleUse;
		for (int id = 2; id < singleUse.field().robotCount(); ++id) {
			for (int upgrade = 0; upgrade < 4; ++upgrade) {
				singleUse.field_.upgradeAbility(id, 0);
			}
		}
		singleUse.computerTurn();
		for (int id = 2; id < singleUse.field().robotCount(); ++id) {
			assert(singleUse.field().robot(id).energy() >= 7);
			if (dynamic_cast<const Teleportation*>(&singleUse.field().robot(id).ability(0))
				&& singleUse.field().distanceTo(id, 1, 1) == Field::NeighborDistance) {
				assert(singleUse.field().robot(id).remainingMovement() == 0);
				assert(singleUse.field().robot(id).energy() == 7);
			}
		}
	}
	assert(sawHealing && sawTeleportation);
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
	Game eof;
	runGame(eof, "");
	assert(eof.field().robot(0).abilityCount() == 0);
	Game initial;
	runGame(initial, "invalid\n2\nq\n");
	assert(initial.field().robot(0).abilityCount() == 1);
	assert(dynamic_cast<const Teleportation*>(&initial.field().robot(0).ability(0)));
	Game target;
	runGame(target, "2\nr\n1\n");
	assert(target.field().robot(0).energy() == 10);
	Game ability;
	runGame(ability, "1\nr\n");
	assert(ability.field().robot(0).energy() == 10);
	Game output;
	std::string text = runGame(output, "2\nr\n1\n1 6\nq\n");
	assert(text.find("нумерация строк и столбцов с 0") != std::string::npos);
	assert(text.find("Телепортация — 5 энергии") != std::string::npos);
	assert(output.field().column(0) == 6);
	assert(output.field().robot(0).energy() == 5);
	Game rewardEof;
	assert(rewardEof.command("1"));
	rewardEof.field_.addRobot(Robot(Robot::Side::Enemy, 1), 1, 2);
	assert(rewardEof.command("d"));
	assert(rewardEof.inputMode_ == Game::InputMode::RankReward);
	runGame(rewardEof, "");
	assert(rewardEof.rewardedRank_ == 1);
}

void TestSuite::run() {
	ownership();
	playerAbilities();
	commandsAndRewards();
	enemies();
	streams();
	std::cout << "All game integration tests passed\n";
}

int main() {
	TestSuite::run();
}
