#pragma once
#include "Field.h"
#include <string>

class InventoryMenu {
public:
	explicit InventoryMenu(Field& field);
	bool active() const;
	void open();
	void handle(const std::string& input, std::string& message);
	std::string prompt() const;
private:
	Field& field_;
	bool active_ = false;
};
