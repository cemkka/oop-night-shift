#pragma once
#include "abilities/Ability.h"

class AreaAttack : public Ability {
public:
	Kind kind() const override;
	std::string name() const override;
	int energyCost() const override;
	bool canUpgrade() const override;
	bool apply(Field& field, int robotId, int targetRow, int targetColumn) override;
	void upgrade() override;
private:
	static constexpr int EnergyCost = 4;
	static constexpr int Damage = 15;
	static constexpr int InitialRadius = 1;
	static constexpr int RadiusIncrease = 1;
	int radius_ = InitialRadius;
};
