#pragma once
#include "Field.h"
#include <string>

class AbilityMenu {
public:
	explicit AbilityMenu(Field& field);
	bool active() const;
	bool rewardsPending() const;
	bool offerRewards();
	void startUse();
	void handle(const std::string& input, std::string& message);
	std::string prompt() const;
private:
	enum class Mode { Initial, Command, Selection, Target, Reward };
	Field& field_;
	Mode mode_ = Mode::Initial;
	int selected_ = 0;
	int awardedRank_ = 1;
	static bool parseNumber(const std::string& input, int& number);
	void choose(const std::string& input, std::string& message);
	void use(const std::string& input, std::string& message);
	void apply(int row, int column, std::string& message);
};
