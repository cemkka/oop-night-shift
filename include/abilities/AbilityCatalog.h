#pragma once
#include "abilities/Ability.h"
#include <memory>

class Robot;

class AbilityCatalog {
public:
	static constexpr int Count = 4;
	static std::unique_ptr<Ability> create(int choice);
	static int find(const Robot& robot, Ability::Kind kind);
	static bool requiresTarget(const Ability& ability);
};
