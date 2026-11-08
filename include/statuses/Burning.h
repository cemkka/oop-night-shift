#pragma once
#include "statuses/Status.h"

class Burning : public Status {
public:
	static constexpr int Damage = 5;
	static constexpr int NoSource = -1;
	explicit Burning(int turns, int sourceId = NoSource);
	std::string name() const override;
	std::unique_ptr<Status> clone() const override;
	bool merge(const Status& other) override;
	bool expired() const override;
	void finishTurn(Robot& robot, Field& field, int id) override;
private:
	int turns_;
	int sourceId_;
};
