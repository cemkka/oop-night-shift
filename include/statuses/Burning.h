#pragma once
#include "statuses/Status.h"

class Burning : public Status {
public:
	static constexpr int Damage = 5;
	explicit Burning(int turns);
	std::string name() const override;
	std::unique_ptr<Status> clone() const override;
	bool merge(const Status& other) override;
	bool expired() const override;
	void finishTurn(Robot& robot) override;
private:
	int turns_;
};
