#include "statuses/Status.h"
#include <algorithm>
#include <limits>

Status::~Status() = default;

void Status::onApply(Robot&) {}

bool Status::blocksMovement() const { return false; }

bool Status::cancelsAbility() { return false; }

int Status::absorbDamage(int amount) { return amount; }

void Status::beginTurn() {}

void Status::finishTurn(Robot&, Field&, int) {}

int Status::saturatedAdd(int value, int amount) {
	return value + std::min(amount, std::numeric_limits<int>::max() - value);
}
