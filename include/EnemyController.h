#pragma once
#include "Field.h"

class EnemyController {
public:
	static void useAbilities(Field& field, int id);
private:
	static bool teleportTarget(const Field& field, int id, int& row, int& column);
};
