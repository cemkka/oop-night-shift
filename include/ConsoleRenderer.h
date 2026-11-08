#pragma once
#include "Field.h"
#include "RobotFactory.h"
#include <string>

class ConsoleRenderer {
public:
	enum class Mode { Standard, Enemies, Impassable, Statuses };
	std::string modeName(Mode mode) const;
	char symbol(const Field& field, const RobotFactory& factory, int row, int column) const;
	std::string styledSymbol(const Field& field, const RobotFactory& factory, int row, int column, Mode mode) const;
	void draw(const Field& field, const RobotFactory& factory, int turn, const std::string& message,
		Mode mode = Mode::Standard) const;
	void message(const std::string& text) const;
private:
	static constexpr int ColumnWidth = 3;
	static constexpr const char* EnemyColor = "\x1b[31m";
	static constexpr const char* ImpassableColor = "\x1b[33m";
	static constexpr const char* StatusColor = "\x1b[36m";
	static constexpr const char* ResetColor = "\x1b[0m";
};
