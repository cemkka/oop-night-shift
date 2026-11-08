#pragma once

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
	bool spendMovement(int amount);
	bool interact(Robot& target);
	void takeDamage(int amount);
	void repair(int amount);
	void gainExperience(int amount);
	void finishTurn();
	void beginTurn();
private:
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
};
