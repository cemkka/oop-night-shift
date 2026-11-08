#include "abilities/AbilityCatalog.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include "abilities/AreaAttack.h"
#include "abilities/RangedAttack.h"
#include "Robot.h"
#include <stdexcept>

std::unique_ptr<Ability> AbilityCatalog::create(int choice) {
	switch (choice) {
	case 1: return std::make_unique<Healing>();
	case 2: return std::make_unique<Teleportation>();
	case 3: return std::make_unique<AreaAttack>();
	case 4: return std::make_unique<RangedAttack>();
	default: throw std::invalid_argument("Invalid ability choice");
	}
}

int AbilityCatalog::find(const Robot& robot, Ability::Kind kind) {
	for (int index = 0; index < robot.abilityCount(); ++index) {
		if (robot.ability(index).kind() == kind) {
			return index;
		}
	}
	return -1;
}

bool AbilityCatalog::requiresTarget(const Ability& ability) {
	return ability.kind() == Ability::Kind::Teleportation || ability.kind() == Ability::Kind::RangedAttack;
}
