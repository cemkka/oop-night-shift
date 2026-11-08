#pragma once
#include "statuses/Status.h"

class Shield : public Status {
public:
	explicit Shield(int protection);
	std::string name() const override;
	std::unique_ptr<Status> clone() const override;
	bool merge(const Status& other) override;
	bool expired() const override;
	int absorbDamage(int amount) override;
private:
	int protection_;
};
