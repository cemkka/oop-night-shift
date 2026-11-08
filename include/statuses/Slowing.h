#pragma once
#include "statuses/Status.h"

class Slowing : public Status {
public:
	explicit Slowing(int turns = 1);
	std::string name() const override;
	std::unique_ptr<Status> clone() const override;
	bool merge(const Status& other) override;
	bool expired() const override;
	bool blocksMovement() const override;
	void onApply(Robot& robot) override;
	void beginTurn() override;
	void finishTurn(Robot& robot, Field& field, int id) override;
private:
	int turns_;
	bool affectedTurn_ = false;
};
