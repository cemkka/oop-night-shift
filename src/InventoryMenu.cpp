#include "InventoryMenu.h"
#include <sstream>

InventoryMenu::InventoryMenu(Field& field) : field_(field) {}

bool InventoryMenu::active() const { return active_; }

void InventoryMenu::open() { active_ = true; }

void InventoryMenu::handle(const std::string& input, std::string& message) {
	int choice = 0;
	std::istringstream stream(input);
	if (!(stream >> choice) || !(stream >> std::ws).eof() || choice < 0 || choice > Inventory::ItemCount) {
		message = "Введите номер предмета, 0 для отмены или Q для выхода.";
		return;
	}
	active_ = false;
	if (choice == 0) {
		message = "Инвентарь закрыт.";
		return;
	}
	auto item = static_cast<Inventory::Item>(choice - 1);
	const Robot& player = field_.robot(Robot::PlayerId);
	if (!player.hasInventory() || player.inventory().count(item) == 0) {
		message = "Такого предмета нет в инвентаре.";
	} else if (field_.useItem(Robot::PlayerId, item)) {
		message = "Предмет использован: " + Inventory::name(item) + ".";
	} else if (item == Inventory::Item::Health || item == Inventory::Item::Energy) {
		message = "Ресурс уже полный. Предмет сохранён.";
	} else {
		message = "Предмет не применён: достигнут предел усиления. Предмет сохранён.";
	}
}

std::string InventoryMenu::prompt() const {
	const Robot& player = field_.robot(Robot::PlayerId);
	if (!player.hasInventory()) {
		return "Инвентарь недоступен. 0 — вернуться; Q — выход.";
	}
	const Inventory& inventory = player.inventory();
	std::string text = "Инвентарь:\n";
	for (int index = 0; index < Inventory::ItemCount; ++index) {
		auto item = static_cast<Inventory::Item>(index);
		text += std::to_string(index + 1) + " — " + Inventory::name(item) + " × "
			+ std::to_string(inventory.count(item)) + ": " + Inventory::description(item) + "\n";
	}
	return text + "Введите номер предмета. 0 — вернуться; Q — выход.";
}
