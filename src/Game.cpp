#include "Game.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <memory>

Game::Game() : field_(MapRows, MapColumns), factory_(field_, 6, 6, FactoryPeriod) {
	field_.setTerrain(3, 4, false);
	field_.setTerrain(3, 5, false);
	field_.setTerrain(1, 3, true, Cell::SlowMovementCost);
	field_.setTerrain(2, 3, true, Cell::SlowMovementCost);
	field_.setTerrain(5, 4, true, Cell::SlowMovementCost);
	field_.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	field_.addRobot(Robot(Robot::Side::Friendly, AllyHealth, AllyAttack, AllySpeed, AllyVision), 2, 1);
	field_.addRobot(Robot(Robot::Side::Enemy, EnemyHealth, EnemyAttack, EnemySpeed, EnemyVision), 2, 6);
	field_.addRobot(Robot(Robot::Side::Enemy, EnemyHealth, EnemyAttack, EnemySpeed, EnemyVision), 5, 7);
	field_.remember(Robot::PlayerId);
}

const Field& Game::field() const { return field_; }

const RobotFactory& Game::factory() const { return factory_; }

int Game::turn() const { return turn_; }

bool Game::lost() const { return !field_.robot(Robot::PlayerId).alive(); }

bool Game::won() const { return !lost() && !factory_.active() && !field_.hasEnemies(); }

bool Game::step(int id, int row, int column) {
	if (!field_.contains(row, column) || !field_.cell(row, column).passable()) {
		if (id == Robot::PlayerId) {
			message_ = "Путь закрыт.";
		}
		return false;
	}
	int target = field_.cell(row, column).robotId();
	bool success;
	if (target != Robot::NoRobotId) {
		success = field_.interact(id, target);
		if (id == Robot::PlayerId) {
			if (!success) {
				message_ = "Взаимодействие невозможно: проверьте энергию, очки и здоровье цели.";
			} else if (field_.robot(target).side() == field_.robot(id).side()) {
				message_ = "Гид отремонтирован.";
			} else if (field_.robot(target).alive()) {
				message_ = "Экспонат повреждён.";
			} else {
				message_ = "Экспонат обезврежен. Получен опыт.";
			}
		} else if (success && target == Robot::PlayerId) {
			message_ = field_.robot(id).side() == Robot::Side::Friendly ? "Гид отремонтировал смотрителя." : "Экспонат атаковал смотрителя.";
		}
	} else {
		success = field_.move(id, row, column);
		if (id == Robot::PlayerId) {
			message_ = success ? "Смотритель переместился." : "Недостаточно очков передвижения.";
		}
	}
	if (id == Robot::PlayerId && success) {
		field_.remember(Robot::PlayerId);
	}
	return success;
}

void Game::enemyAbility(int id) {
	const Robot& enemy = field_.robot(id);
	if (enemy.side() != Robot::Side::Enemy) {
		return;
	}
	for (int index = 0; index < enemy.abilityCount(); ++index) {
		const Ability& ability = enemy.ability(index);
		if (dynamic_cast<const Healing*>(&ability)) {
			if (enemy.health() < enemy.maxHealth()) {
				field_.applyAbility(id, index, 0, 0);
			}
			return;
		}
		if (!dynamic_cast<const Teleportation*>(&ability)
			|| enemy.energy() < ability.energyCost()
			|| !field_.visible(id, field_.row(Robot::PlayerId), field_.column(Robot::PlayerId))) {
			continue;
		}
		int bestDistance = field_.distanceTo(id, field_.row(Robot::PlayerId), field_.column(Robot::PlayerId));
		int bestRow = -1;
		int bestColumn = -1;
		if (bestDistance <= Field::NeighborDistance) {
			return;
		}
		for (int row = 0; row < field_.rows(); ++row) {
			for (int column = 0; column < field_.columns(); ++column) {
				if (!field_.free(row, column) || field_.distanceTo(id, row, column) > Teleportation::Radius) {
					continue;
				}
				int distance = field_.distanceTo(Robot::PlayerId, row, column);
				if (distance < bestDistance) {
					bestDistance = distance;
					bestRow = row;
					bestColumn = column;
				}
			}
		}
		if (bestRow >= 0) {
			field_.applyAbility(id, index, bestRow, bestColumn);
		}
		return;
	}
}

void Game::computerTurn() {
	const int rowOffsets[] = {-1, 1, 0, 0};
	const int columnOffsets[] = {0, 0, -1, 1};
	static_assert(std::size(rowOffsets) == std::size(columnOffsets));
	int count = field_.robotCount();
	for (int id = 0; id < count && !lost(); ++id) {
		if (id == Robot::PlayerId || !field_.robot(id).alive()) {
			continue;
		}
		field_.beginTurn(id);
		enemyAbility(id);
		while (field_.robot(id).remainingMovement() > 0 && !lost()) {
			if (!field_.visible(id, field_.row(Robot::PlayerId), field_.column(Robot::PlayerId))) {
				break;
			}
			int distance = field_.distanceTo(id, field_.row(Robot::PlayerId), field_.column(Robot::PlayerId));
			if (distance == Field::NeighborDistance) {
				if (!step(id, field_.row(Robot::PlayerId), field_.column(Robot::PlayerId))) {
					break;
				}
				continue;
			}

			if (field_.robot(id).side() == Robot::Side::Friendly) {
				break;
			}
			bool moved = false;
			for (std::size_t direction = 0; direction < std::size(rowOffsets); ++direction) {
				int row = field_.row(id) + rowOffsets[direction];
				int column = field_.column(id) + columnOffsets[direction];
				if (!field_.free(row, column)) {
					continue;
				}
				int nextDistance = field_.distanceTo(Robot::PlayerId, row, column);
				if (nextDistance < distance && step(id, row, column)) {
					moved = true;
					break;
				}
			}
			if (!moved) {
				break;
			}
		}
	}
}

void Game::finishTurn() {
	computerTurn();
	if (lost()) {
		return;
	}

	factory_.advance();
	for (int id = 0; id < field_.robotCount(); ++id) {
		field_.finishTurn(id);
	}
	field_.beginTurn(Robot::PlayerId);
	++turn_;
	field_.remember(Robot::PlayerId);
}

bool Game::parseNumber(const std::string& input, int& number) {
	std::istringstream stream(input);
	return (stream >> number) && (stream >> std::ws).eof();
}

int Game::findAbility(bool teleportation) const {
	const Robot& player = field_.robot(Robot::PlayerId);
	for (int index = 0; index < player.abilityCount(); ++index) {
		const Ability& ability = player.ability(index);
		if ((teleportation && dynamic_cast<const Teleportation*>(&ability))
			|| (!teleportation && dynamic_cast<const Healing*>(&ability))) {
			return index;
		}
	}
	return -1;
}

bool Game::chooseAbility(const std::string& input) {
	int choice = 0;
	if (!parseNumber(input, choice) || (choice != 1 && choice != 2)) {
		message_ = "Введите 1 или 2; Q — выход.";
		return true;
	}
	int index = findAbility(choice == 2);
	if (index < 0) {
		if (choice == 1) {
			field_.addAbility(Robot::PlayerId, std::make_unique<Healing>());
		} else {
			field_.addAbility(Robot::PlayerId, std::make_unique<Teleportation>());
		}
		message_ = "Способность получена.";
	} else {
		const auto* teleportation = dynamic_cast<const Teleportation*>(&field_.robot(Robot::PlayerId).ability(index));
		if (teleportation && !teleportation->canUpgrade()) {
			message_ = "Телепортация уже улучшена до предела. Выберите лечение; награда сохранена.";
			return true;
		}
		field_.upgradeAbility(Robot::PlayerId, index);
		message_ = "Способность улучшена.";
	}
	if (inputMode_ == InputMode::InitialAbility) {
		inputMode_ = InputMode::Command;
	} else {
		++rewardedRank_;
		if (rewardedRank_ >= field_.robot(Robot::PlayerId).rank()) {
			inputMode_ = InputMode::Command;
			if (endTurnPending_) {
				endTurnPending_ = false;
				finishTurn();
			}
		}
	}
	return true;
}

void Game::applyPlayerAbility(int row, int column) {
	bool success = field_.applyAbility(Robot::PlayerId, selectedAbility_, row, column);
	message_ = success ? "Способность применена." : "Способность не применена: проверьте цель, здоровье и энергию.";
	inputMode_ = InputMode::Command;
}

bool Game::useAbility(const std::string& input) {
	if (inputMode_ == InputMode::Target) {
		int row = 0;
		int column = 0;
		std::istringstream stream(input);
		if (!(stream >> row >> column) || !(stream >> std::ws).eof()) {
			message_ = "Введите два целых числа: строку и столбец; Q — выход.";
			return true;
		}
		applyPlayerAbility(row, column);
		return true;
	}
	int choice = 0;
	const Robot& player = field_.robot(Robot::PlayerId);
	if (!parseNumber(input, choice) || choice < 1 || choice > player.abilityCount()) {
		message_ = "Неверный номер способности; Q — выход.";
		return true;
	}
	selectedAbility_ = choice - 1;
	if (dynamic_cast<const Teleportation*>(&player.ability(selectedAbility_))) {
		inputMode_ = InputMode::Target;
		message_ = "Выберите цель телепортации.";
	} else {
		applyPlayerAbility(0, 0);
	}
	return true;
}

void Game::prompt() const {
	if (inputMode_ == InputMode::InitialAbility) {
		renderer_.message("Начальная способность: 1 — лечение (" + std::to_string(Healing().energyCost())
			+ " энергии); 2 — телепортация (" + std::to_string(Teleportation().energyCost()) + " энергии). Q — выход.");
	} else if (inputMode_ == InputMode::RankReward) {
		renderer_.message("Награда за ранг " + std::to_string(rewardedRank_ + 1) + ":");
		for (int choice = 1; choice <= 2; ++choice) {
			int index = findAbility(choice == 2);
			std::string text = std::to_string(choice) + (choice == 1 ? " — лечение: " : " — телепортация: ");
			if (index < 0) {
				text += "получить";
			} else {
				const auto* teleportation = dynamic_cast<const Teleportation*>(&field_.robot(Robot::PlayerId).ability(index));
				text += teleportation && !teleportation->canUpgrade() ? "предел улучшений" : "улучшить";
			}
			renderer_.message(text);
		}
		renderer_.message("Введите номер; Q — выход.");
	} else if (inputMode_ == InputMode::Ability) {
		renderer_.message("Введите номер способности из списка; Q — выход.");
	} else if (inputMode_ == InputMode::Target) {
		renderer_.message("Введите строку и столбец через пробел. Нумерация с 0; Q — выход.");
	}
}

bool Game::command(const std::string& input) {
	if (input == "q" || input == "Q") {
		return false;
	}
	if (lost() || (won() && inputMode_ != InputMode::RankReward)) {
		return false;
	}
	if (inputMode_ == InputMode::InitialAbility || inputMode_ == InputMode::RankReward) {
		return chooseAbility(input);
	}
	if (inputMode_ == InputMode::Ability || inputMode_ == InputMode::Target) {
		return useAbility(input);
	}
	if (input.size() != CommandLength) {
		message_ = "Введите одну английскую букву команды.";
		return true;
	}
	char key = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
	if (key == 'r') {
		inputMode_ = InputMode::Ability;
		message_ = "Выберите способность.";
		return true;
	}
	if (key == 'e') {
		message_ = "Ход завершён.";
		finishTurn();
		return true;
	}
	if (key == 'f') {
		if (!factory_.active() || !factory_.adjacent(field_.row(Robot::PlayerId), field_.column(Robot::PlayerId))) {
			message_ = "Нужен действующий станок в соседней клетке.";
		} else if (field_.spendMovement(Robot::PlayerId, ShutdownCost)) {
			factory_.disable(field_.row(Robot::PlayerId), field_.column(Robot::PlayerId));
			message_ = "Реставрационный станок отключён.";
		}
	} else if (key == 'w' || key == 's' || key == 'a' || key == 'd') {
		int row = field_.row(Robot::PlayerId);
		int column = field_.column(Robot::PlayerId);
		switch (key) {
		case 'w':
			--row;
			break;
		case 's':
			++row;
			break;
		case 'a':
			--column;
			break;
		case 'd':
			++column;
			break;
		}
		step(Robot::PlayerId, row, column);
	} else {
		message_ = "Неизвестная команда. Используйте английскую раскладку.";
	}
	if (field_.robot(Robot::PlayerId).rank() > rewardedRank_) {
		inputMode_ = InputMode::RankReward;
		endTurnPending_ = !won() && field_.robot(Robot::PlayerId).remainingMovement() == 0;
	} else if (!won() && field_.robot(Robot::PlayerId).remainingMovement() == 0) {
		finishTurn();
	}
	return true;
}

void Game::run() {
	while (true) {
		renderer_.draw(field_, factory_, turn_, message_);
		if (won() && inputMode_ != InputMode::RankReward) {
			renderer_.message("Победа! Музей в безопасности.");
			break;
		}
		if (lost()) {
			renderer_.message("Поражение. Смотритель вышел из строя.");
			break;
		}
		std::string input;
		prompt();
		if (!std::getline(std::cin, input) || !command(input)) {
			renderer_.message("Смена завершена.");
			break;
		}
	}
}
