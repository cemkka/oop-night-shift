#pragma once
#include "abilities/Ability.h"

class Teleportation : public Ability {
public:
	std::string name() const override;
	int energyCost() const override;
	bool apply(Field& field, int robotId, int targetRow, int targetColumn) override;
	void upgrade() override;
private:
	static constexpr int Radius = 5;
	static constexpr int InitialEnergyCost = 5;
	static constexpr int EnergyCostDecrease = 1;
	static constexpr int MinimumEnergyCost = 1;

	int energyCost_ = InitialEnergyCost;
};
