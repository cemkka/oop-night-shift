#pragma once
#include "abilities/Ability.h"

class Healing : public Ability {
public:
	std::string name() const override;
	int energyCost() const override;
	bool apply(Field& field, int robotId, int targetRow, int targetColumn) override;
	void upgrade() override;
private:
	static constexpr int EnergyCost = 3;
	static constexpr int InitialHealing = 20;
	static constexpr int HealingIncrease = 10;

	int healing_ = InitialHealing;
};
