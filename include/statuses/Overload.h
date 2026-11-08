#pragma once
#include "statuses/Status.h"

class Overload : public Status {
public:
	std::string name() const override;
	std::unique_ptr<Status> clone() const override;
	bool merge(const Status& other) override;
	bool expired() const override;
	bool cancelsAbility() override;
private:
	bool active_ = true;
};
