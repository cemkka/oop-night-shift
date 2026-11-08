#pragma once
#include "abilities/Ability.h"

class RangedAttack : public Ability {
public:
	Kind kind() const override;
	std::string name() const override;
	int energyCost() const override;
	bool canUpgrade() const override;
	bool apply(Field& field, int robotId, int targetRow, int targetColumn) override;
	void upgrade() override;
private:
	static constexpr int Radius = 3;
	static constexpr int EnergyCost = 3;
	static constexpr int InitialDamage = 20;
	static constexpr int DamageIncrease = 10;
	int damage_ = InitialDamage;
};
