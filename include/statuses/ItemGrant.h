#pragma once
#include "statuses/Status.h"
#include "Inventory.h"

class ItemGrant : public Status {
public:
	explicit ItemGrant(Inventory::Item item);
	std::string name() const override;
	std::unique_ptr<Status> clone() const override;
	bool merge(const Status& other) override;
	bool expired() const override;
	void onApply(Robot& robot) override;
private:
	Inventory::Item item_;
	bool consumed_ = false;
};
