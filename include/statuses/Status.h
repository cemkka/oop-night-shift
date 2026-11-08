#pragma once
#include <memory>
#include <string>

class Robot;

class Status {
public:
	virtual ~Status() = 0;
	virtual std::string name() const = 0;
	virtual std::unique_ptr<Status> clone() const = 0;
	virtual bool merge(const Status& other) = 0;
	virtual bool expired() const = 0;
	virtual bool blocksMovement() const;
	virtual bool cancelsAbility();
	virtual int absorbDamage(int amount);
	virtual void beginTurn();
	virtual void finishTurn(Robot& robot);
protected:
	static int saturatedAdd(int value, int amount);
};
