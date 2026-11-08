#include "Game.h"
#include "EnemyController.h"
#include <cctype>
#include <iostream>
#include <iterator>

Game::Game(bool populateStatuses)
	: field_(MapRows, MapColumns, populateStatuses), factory_(field_, 6, 6, FactoryPeriod), menu_(field_) {
	field_.setTerrain(3, 4, false);
	field_.setTerrain(3, 5, false);
	field_.setTerrain(1, 3, true, Cell::SlowMovementCost);
	field_.setTerrain(2, 3, true, Cell::SlowMovementCost);
	field_.setTerrain(5, 4, true, Cell::SlowMovementCost);
	field_.addRobot(Robot(Robot::Side::Friendly), 1, 1);
	field_.addRobot(Robot(Robot::Side::Friendly, AllyHealth, AllyAttack, AllySpeed, AllyVision), 2, 1);
	field_.addRobot(Robot(Robot::Side::Enemy, EnemyHealth, EnemyAttack, EnemySpeed, EnemyVision), 2, 6);
	field_.addRobot(Robot(Robot::Side::Enemy, EnemyHealth, EnemyAttack, EnemySpeed, EnemyVision), 5, 7);
	field_.beginTurn(Robot::PlayerId);
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
			message_ = success ? "Смотритель переместился." : "Шаг невозможен: проверьте очки движения и замедление.";
		}
	}
	if (id == Robot::PlayerId && success) {
		field_.remember(Robot::PlayerId);
	}
	return success;
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
		EnemyController::useAbilities(field_, id);
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

	for (int id = 0; id < field_.robotCount(); ++id) {
		field_.finishTurn(id);
	}
	if (lost()) {
		return;
	}
	factory_.advance();
	field_.beginTurn(Robot::PlayerId);
	++turn_;
	field_.remember(Robot::PlayerId);
}

bool Game::command(const std::string& input) {
	if (input == "q" || input == "Q") {
		return false;
	}
	if (lost() || (won() && !menu_.rewardsPending())) {
		return false;
	}
	if (menu_.active()) {
		menu_.handle(input, message_);
		if (endTurnPending_ && !menu_.active()) {
			endTurnPending_ = false;
			finishTurn();
		}
		return true;
	}
	if (input.size() != CommandLength) {
		message_ = "Введите одну английскую букву команды.";
		return true;
	}
	char key = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
	if (key == 'r') {
		menu_.startUse();
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
	if (menu_.offerRewards()) {
		endTurnPending_ = !won() && field_.robot(Robot::PlayerId).remainingMovement() == 0;
	} else if (!won() && field_.robot(Robot::PlayerId).remainingMovement() == 0) {
		finishTurn();
	}
	return true;
}

void Game::run() {
	while (true) {
		renderer_.draw(field_, factory_, turn_, message_);
		if (won() && !menu_.rewardsPending()) {
			renderer_.message("Победа! Музей в безопасности.");
			break;
		}
		if (lost()) {
			renderer_.message("Поражение. Смотритель вышел из строя.");
			break;
		}
		std::string input;
		if (menu_.active()) {
			renderer_.message(menu_.prompt());
		}
		if (!std::getline(std::cin, input) || !command(input)) {
			renderer_.message("Смена завершена.");
			break;
		}
	}
}
