#pragma once
#include "abilities/Ability.h"
#include "statuses/Status.h"
#include "Inventory.h"
#include <memory>
#include <vector>

class Robot {
public:
	static constexpr int PlayerId = 0;
	static constexpr int NoRobotId = -1;
	static constexpr int DefaultHealth = 100;
	static constexpr int DefaultAttack = 25;
	static constexpr int DefaultSpeed = 3;
	static constexpr int DefaultVision = 4;

	enum class Side { Friendly, Enemy };
	Robot(Side side, int health = DefaultHealth, int attack = DefaultAttack,
		int speed = DefaultSpeed, int vision = DefaultVision);
	Robot(const Robot&) = delete;
	Robot& operator=(const Robot&) = delete;
	Robot(Robot&&) noexcept = default;
	Robot& operator=(Robot&&) noexcept = default;
	int abilityCount() const;
	const Ability& ability(int index) const;
	void addAbility(std::unique_ptr<Ability> ability);
	void upgradeAbility(int index);
	bool applyAbility(int index, Field& field, int id, int row, int column);
	int statusCount() const;
	const Status& status(int index) const;
	void addStatus(std::unique_ptr<Status> status);
	bool canMove() const;
	bool prepareAbility();
	const Inventory* inventory() const;
	bool receiveItem(Inventory::Item item);
	bool useItem(Inventory::Item item);
	void damageFrom(Robot& actor, int amount);
	int health() const;
	int maxHealth() const;
	int energy() const;
	int maxEnergy() const;
	int attack() const;
	int experience() const;
	int experienceToNextRank() const;
	int rank() const;
	int speed() const;
	int vision() const;
	int remainingMovement() const;
	Side side() const;
	bool alive() const;
	bool spendEnergy(int amount);
	bool spendMovement(int amount);
	bool interact(Robot& target);
	void takeDamage(int amount);
	void repair(int amount);
	void restoreEnergy(int amount);
	void gainExperience(int amount);
	void finishTurn();
	void beginTurn();
private:
	friend class Field;
	void setPlayer(bool player);
	static constexpr int InitialRank = 1;
	static constexpr int InitialEnergy = 10;
	static constexpr int InitialExperienceToNextRank = 20;
	static constexpr int InteractionEnergyCost = 2;
	static constexpr int InteractionMovementCost = 1;
	static constexpr int EnergyRestoredPerTurn = 3;
	static constexpr int ExperiencePerDefeatedEnemy = 20;
	static constexpr int ExperienceRequirementIncrease = 10;
	static constexpr int HealthIncreasePerRank = 10;
	static constexpr int AttackIncreasePerRank = 5;
	static constexpr int EnergyIncreasePerRank = 2;

	int cappedAdd(int value, int amount) const;
	Side side_;
	int health_, maxHealth_, attack_;
	int energy_ = InitialEnergy, maxEnergy_ = InitialEnergy;
	int experience_ = 0, experienceToNextRank_ = InitialExperienceToNextRank, rank_ = InitialRank;
	int speed_, vision_, remainingMovement_;
	std::vector<std::unique_ptr<Ability>> abilities_;
	std::vector<std::unique_ptr<Status>> statuses_;
	std::unique_ptr<Inventory> inventory_;
	void removeExpiredStatuses();
};
