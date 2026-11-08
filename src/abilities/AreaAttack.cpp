#include "abilities/AreaAttack.h"
#include "Field.h"
#include "statuses/Overload.h"
#include <algorithm>
#include <limits>

Ability::Kind AreaAttack::kind() const { return Kind::AreaAttack; }

std::string AreaAttack::name() const { return "Удар вокруг"; }

int AreaAttack::energyCost() const { return EnergyCost; }

bool AreaAttack::canUpgrade() const { return radius_ < std::numeric_limits<int>::max(); }

bool AreaAttack::apply(Field& field, int robotId, int, int) {
	if (!field.prepareAbility(robotId)) {
		return false;
	}
	bool hasTarget = false;
	for (int id = 0; id < field.robotCount(); ++id) {
		if (id != robotId && field.robot(id).alive()
			&& field.distanceTo(robotId, field.row(id), field.column(id)) <= radius_) {
			hasTarget = true;
			break;
		}
	}
	if (!hasTarget || !field.spendEnergy(robotId, energyCost())) {
		return false;
	}
	for (int id = 0; id < field.robotCount(); ++id) {
		if (id != robotId && field.robot(id).alive()
			&& field.distanceTo(robotId, field.row(id), field.column(id)) <= radius_) {
			field.damageFrom(robotId, id, Damage);
			if (field.robot(id).alive()) {
				field.addStatus(id, std::make_unique<Overload>());
			}
		}
	}
	return true;
}

void AreaAttack::upgrade() {
	radius_ += std::min(RadiusIncrease, std::numeric_limits<int>::max() - radius_);
}
