#pragma once
#include <string>

class Field;

class Ability {
public:
	enum class Kind { Healing, Teleportation, AreaAttack, RangedAttack };
	virtual ~Ability() = 0;
	virtual Kind kind() const = 0;
	virtual bool canUpgrade() const = 0;
	virtual std::string name() const = 0;
	virtual int energyCost() const = 0;
	virtual bool apply(Field& field, int robotId, int targetRow, int targetColumn) = 0;
	virtual void upgrade() = 0;
};
