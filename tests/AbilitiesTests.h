#pragma once
#include <vector>

class Ability;
class Field;

class AbilityTests {
public:
	static void run();
private:
	static std::vector<int> snapshot(const Field& field);
	static void expectFailure(Ability& ability, Field& field, int id, int row, int column);
	static void testHealing();
	static void testTeleportation();
	static void testDeadRobotsAndEnergy();
	static void testMovementPreserved();
	static void testAreaStatuses();
	static void testRangedStatuses();
	static void testHealingShield();
	static void testRepairWithoutShield();
};
