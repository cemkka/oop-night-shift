#pragma once
#include "Field.h"
#include "RobotFactory.h"
#include <string>

class ConsoleRenderer {
public:
	char symbol(const Field& field, const RobotFactory& factory, int row, int column) const;
	void draw(const Field& field, const RobotFactory& factory, int turn, const std::string& message) const;
	void message(const std::string& text) const;
};
