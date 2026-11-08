#include "statuses/ItemGrant.h"
#include "Robot.h"
#include <stdexcept>

ItemGrant::ItemGrant(Inventory::Item item) : item_(item) {
	if (!Inventory::valid(item)) {
		throw std::invalid_argument("Invalid granted item");
	}
}

std::string ItemGrant::name() const { return "Предмет: " + Inventory::name(item_); }

std::unique_ptr<Status> ItemGrant::clone() const { return std::make_unique<ItemGrant>(*this); }

bool ItemGrant::merge(const Status&) { return false; }

bool ItemGrant::expired() const { return consumed_; }

void ItemGrant::onApply(Robot& robot) {
	if (consumed_) {
		return;
	}
	consumed_ = true;
	robot.receiveItem(item_);
}
