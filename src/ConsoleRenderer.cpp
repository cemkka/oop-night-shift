#include "ConsoleRenderer.h"
#include <iostream>
#include <iomanip>

std::string ConsoleRenderer::modeName(Mode mode) const {
	switch (mode) {
	case Mode::Standard: return "Стандартный";
	case Mode::Enemies: return "Подсветка врагов";
	case Mode::Impassable: return "Подсветка непроходимых клеток";
	case Mode::Statuses: return "Подсветка клеток со статусами";
	}
	return "Стандартный";
}

char ConsoleRenderer::symbol(const Field& field, const RobotFactory& factory, int row, int column) const {
	const Cell& cell = field.cell(row, column);
	bool visible = field.visible(Robot::PlayerId, row, column);
	bool rememberedTerrain = cell.known() && (!cell.passable() || cell.cost() == Cell::SlowMovementCost);
	if (!visible && !rememberedTerrain) {
		return '?';
	}
	if (visible) {
		int id = cell.robotId();
		if (id == Robot::PlayerId) {
			return '@';
		}
		if (id != Robot::NoRobotId) {
			return field.robot(id).side() == Robot::Side::Enemy ? 'E' : '+';
		}
	}
	if (factory.occupies(row, column)) {
		return 'F';
	}
	if (!cell.passable()) {
		return '#';
	}
	if (cell.cost() == Cell::SlowMovementCost) {
		return '~';
	}
	return '.';
}

std::string ConsoleRenderer::styledSymbol(const Field& field, const RobotFactory& factory,
	int row, int column, Mode mode) const {
	char value = symbol(field, factory, row, column);
	const char* color = nullptr;
	switch (mode) {
	case Mode::Standard: break;
	case Mode::Enemies:
		if (value == 'E') {
			color = EnemyColor;
		}
		break;
	case Mode::Impassable:
		if (value == '#' || value == 'F') {
			color = ImpassableColor;
		}
		break;
	case Mode::Statuses:
		if (field.visible(Robot::PlayerId, row, column) && field.cell(row, column).statusCount() > 0) {
			color = StatusColor;
		}
		break;
	}
	return color ? std::string(color) + value + ResetColor : std::string(1, value);
}

void ConsoleRenderer::draw(const Field& field, const RobotFactory& factory, int turn, const std::string& text,
	Mode mode) const {
	const Robot& player = field.robot(Robot::PlayerId);
	std::cout << "\nНочная смена — ход " << turn << "\n";
	std::cout << "Режим карты: " << modeName(mode) << '\n';
	std::cout << "Координаты: нумерация строк и столбцов с 0.\n    ";
	for (int column = 0; column < field.columns(); ++column) {
		std::cout << std::setw(3) << column;
	}
	std::cout << '\n';
	for (int row = 0; row < field.rows(); ++row) {
		std::cout << std::setw(3) << row << ' ';
		for (int column = 0; column < field.columns(); ++column) {
			std::cout << "  " << styledSymbol(field, factory, row, column, mode);
		}
		std::cout << '\n';
	}
	std::cout << "Здоровье: " << player.health() << '/' << player.maxHealth()
		<< "  Энергия: " << player.energy() << '/' << player.maxEnergy()
		<< "\nОпыт: " << player.experience() << '/' << player.experienceToNextRank()
		<< "  Ранг: " << player.rank() << "  Атака: " << player.attack()
		<< "  Бонус атаки: +" << (player.inventory() ? player.inventory()->attackBonus() : 0)
		<< " (ходов: " << (player.inventory() ? player.inventory()->attackTurns() : 0) << ')'
		<< "\nПередвижение: " << player.remainingMovement() << '/' << player.speed()
		<< "  Обзор: " << player.vision()
		<< "\n@ смотритель, E враг, + гид, # стена, . пол, ~ завалы, ? неизвестно, F станок"
		<< "\n" << text
		<< "\nW/A/S/D + Enter: шаг или взаимодействие; E: конец хода; F: отключить станок; R: способность; I: инвентарь; V: режим карты; Q: выход\n";
	std::cout << "Способности:\n";
	for (int index = 0; index < player.abilityCount(); ++index) {
		const Ability& ability = player.ability(index);
		std::cout << index + 1 << ": " << ability.name() << " — " << ability.energyCost() << " энергии\n";
	}
	std::cout << "Статусы роботов в обзоре:\n";
	for (int id = 0; id < field.robotCount(); ++id) {
		if (!field.robot(id).alive() || !field.visible(Robot::PlayerId, field.row(id), field.column(id))) {
			continue;
		}
		const Robot& robot = field.robot(id);
		std::cout << "ID " << id << " (" << field.row(id) << ", " << field.column(id) << "): ";
		if (robot.statusCount() == 0) {
			std::cout << "нет";
		}
		for (int index = 0; index < robot.statusCount(); ++index) {
			std::cout << (index == 0 ? "" : ", ") << robot.status(index).name();
		}
		std::cout << '\n';
	}
}

void ConsoleRenderer::message(const std::string& text) const { std::cout << text << '\n'; }
