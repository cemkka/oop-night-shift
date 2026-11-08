#include "Robot.h"
#include "abilities/Healing.h"
#include "abilities/Teleportation.h"
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <random>
#include <utility>

Robot::Robot(Side side, int health, int attack, int speed, int vision)
	: side_(side), health_(health), maxHealth_(health), attack_(attack),
	speed_(speed), vision_(vision), remainingMovement_(speed) {
	if (health <= 0 || attack <= 0 || speed <= 0 || vision <= 0
		|| (side != Side::Friendly && side != Side::Enemy)) {
		throw std::invalid_argument("Invalid robot characteristics");
	}
	if (side_ == Side::Enemy) {
		static std::mt19937 generator(std::random_device{}());
		if (std::uniform_int_distribution<int>(0, 1)(generator) == 0) {
			addAbility(std::make_unique<Healing>());
		} else {
			addAbility(std::make_unique<Teleportation>());
		}
	}
}

int Robot::abilityCount() const { return static_cast<int>(abilities_.size()); }

const Ability& Robot::ability(int index) const { return *abilities_.at(index); }

void Robot::addAbility(std::unique_ptr<Ability> value) {
	if (!value) {
		throw std::invalid_argument("Null ability");
	}
	abilities_.push_back(std::move(value));
}

void Robot::upgradeAbility(int index) { abilities_.at(index)->upgrade(); }

bool Robot::applyAbility(int index, Field& field, int id, int row, int column) {
	return alive() && index >= 0 && index < abilityCount()
		&& abilities_[index]->apply(field, id, row, column);
}

int Robot::health() const { return health_; }

int Robot::maxHealth() const { return maxHealth_; }

int Robot::energy() const { return energy_; }

int Robot::maxEnergy() const { return maxEnergy_; }

int Robot::attack() const { return attack_; }

int Robot::experience() const { return experience_; }

int Robot::experienceToNextRank() const { return experienceToNextRank_; }

int Robot::rank() const { return rank_; }

int Robot::speed() const { return speed_; }

int Robot::vision() const { return vision_; }

int Robot::remainingMovement() const { return remainingMovement_; }

Robot::Side Robot::side() const { return side_; }

bool Robot::alive() const { return health_ > 0; }

bool Robot::spendEnergy(int amount) {
	if (!alive() || amount <= 0 || energy_ < amount) {
		return false;
	}
	energy_ -= amount;
	return true;
}

bool Robot::spendMovement(int amount) {
	if (!alive() || amount <= 0 || remainingMovement_ < amount) {
		return false;
	}
	remainingMovement_ -= amount;
	return true;
}

bool Robot::interact(Robot& target) {
	if (this == &target || !alive() || !target.alive() || energy_ < InteractionEnergyCost) {
		return false;
	}
	if (side_ == target.side_ && target.health_ == target.maxHealth_) {
		return false;
	}
	if (!spendMovement(InteractionMovementCost)) {
		return false;
	}
	energy_ -= InteractionEnergyCost;
	if (side_ == target.side_) {
		target.repair(attack_);
	} else {
		target.takeDamage(attack_);
		if (!target.alive()) {
			gainExperience(ExperiencePerDefeatedEnemy);
		}
	}
	return true;
}

void Robot::takeDamage(int amount) {
	health_ -= std::min(health_, std::max(0, amount));
}

void Robot::repair(int amount) {
	if (alive()) {
		health_ += std::min(maxHealth_ - health_, std::max(0, amount));
	}
}

void Robot::gainExperience(int amount) {
	if (amount <= 0 || !alive()) {
		return;
	}
	long long total = static_cast<long long>(experience_) + amount;
	while (total >= experienceToNextRank_) {
		total -= experienceToNextRank_;
		experienceToNextRank_ = cappedAdd(experienceToNextRank_, ExperienceRequirementIncrease);
		rank_ = cappedAdd(rank_, 1);
		maxHealth_ = cappedAdd(maxHealth_, HealthIncreasePerRank);
		health_ = cappedAdd(health_, HealthIncreasePerRank);
		attack_ = cappedAdd(attack_, AttackIncreasePerRank);
		maxEnergy_ = cappedAdd(maxEnergy_, EnergyIncreasePerRank);
		energy_ = cappedAdd(energy_, EnergyIncreasePerRank);
	}
	experience_ = static_cast<int>(total);
}

int Robot::cappedAdd(int value, int amount) const {
	const int limit = std::numeric_limits<int>::max();
	return value + std::min(amount, limit - value);
}

void Robot::finishTurn() {
	if (alive()) {
		energy_ += std::min(EnergyRestoredPerTurn, maxEnergy_ - energy_);
	}
}

void Robot::beginTurn() { remainingMovement_ = speed_; }
