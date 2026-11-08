#include "ConsoleRenderer.h"
#include <iostream>

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

void ConsoleRenderer::draw(const Field& field, const RobotFactory& factory, int turn, const std::string& text) const {
	const Robot& player = field.robot(Robot::PlayerId);
	std::cout << "\nНочная смена — ход " << turn << "\n";
	for (int row = 0; row < field.rows(); ++row) {
		for (int column = 0; column < field.columns(); ++column) {
			std::cout << symbol(field, factory, row, column) << ' ';
		}
		std::cout << '\n';
	}
	std::cout << "Здоровье: " << player.health() << '/' << player.maxHealth()
		<< "  Энергия: " << player.energy() << '/' << player.maxEnergy()
		<< "\nОпыт: " << player.experience() << '/' << player.experienceToNextRank()
		<< "  Ранг: " << player.rank() << "  Атака: " << player.attack()
		<< "\nПередвижение: " << player.remainingMovement() << '/' << player.speed()
		<< "  Обзор: " << player.vision()
		<< "\n@ смотритель, E враг, + гид, # стена, . пол, ~ завалы, ? неизвестно, F станок"
		<< "\n" << text
		<< "\nW/A/S/D + Enter: шаг или взаимодействие; E: конец хода; F: отключить станок; Q: выход\n> ";
}

void ConsoleRenderer::message(const std::string& text) const { std::cout << text << '\n'; }
