#include "AbilityMenu.h"
#include "abilities/AbilityCatalog.h"
#include "statuses/Overload.h"
#include <sstream>

AbilityMenu::AbilityMenu(Field& field) : field_(field) {}

bool AbilityMenu::active() const { return mode_ != Mode::Command; }

bool AbilityMenu::rewardsPending() const { return mode_ == Mode::Reward; }

bool AbilityMenu::offerRewards() {
	if (field_.robot(Robot::PlayerId).rank() > awardedRank_) {
		mode_ = Mode::Reward;
		return true;
	}
	return false;
}

void AbilityMenu::startUse() { mode_ = Mode::Selection; }

bool AbilityMenu::parseNumber(const std::string& input, int& number) {
	std::istringstream stream(input);
	return (stream >> number) && (stream >> std::ws).eof();
}

void AbilityMenu::handle(const std::string& input, std::string& message) {
	if (mode_ == Mode::Initial || mode_ == Mode::Reward) {
		choose(input, message);
	} else {
		use(input, message);
	}
}

void AbilityMenu::choose(const std::string& input, std::string& message) {
	int choice = 0;
	if (!parseNumber(input, choice) || choice < 1 || choice > AbilityCatalog::Count) {
		message = "Введите номер от 1 до 4; Q — выход.";
		return;
	}
	auto ability = AbilityCatalog::create(choice);
	int index = AbilityCatalog::find(field_.robot(Robot::PlayerId), ability->kind());
	if (index < 0) {
		field_.addAbility(Robot::PlayerId, std::move(ability));
		message = "Способность получена.";
	} else {
		if (!field_.robot(Robot::PlayerId).ability(index).canUpgrade()) {
			message = "Достигнут предел улучшения. Награда сохранена; выберите другую способность.";
			return;
		}
		field_.upgradeAbility(Robot::PlayerId, index);
		message = "Способность улучшена.";
	}
	bool reward = mode_ == Mode::Reward;
	mode_ = Mode::Command;
	if (reward) {
		++awardedRank_;
		offerRewards();
	}
}

void AbilityMenu::apply(int row, int column, std::string& message) {
	const Robot& player = field_.robot(Robot::PlayerId);
	const Ability& ability = player.ability(selected_);
	bool overloaded = false;
	for (int index = 0; index < player.statusCount(); ++index) {
		overloaded = overloaded || dynamic_cast<const Overload*>(&player.status(index)) != nullptr;
	}
	bool success = field_.applyAbility(Robot::PlayerId, selected_, row, column);
	if (success) {
		message = "Способность применена.";
	} else if (overloaded) {
		message = "Перегрузка отменила способность и снялась. Энергия не потрачена.";
	} else if (player.energy() < ability.energyCost()) {
		message = "Недостаточно энергии.";
	} else if (ability.kind() == Ability::Kind::Teleportation && !player.canMove()) {
		message = "Замедление запрещает телепортацию.";
	} else {
		message = "Недопустимая цель: проверьте границы и радиус. Для удара нужен другой робот, для телепортации — свободная проходимая клетка.";
	}
	mode_ = Mode::Command;
	offerRewards();
}

void AbilityMenu::use(const std::string& input, std::string& message) {
	if (mode_ == Mode::Target) {
		int row = 0;
		int column = 0;
		std::istringstream stream(input);
		if (!(stream >> row >> column) || !(stream >> std::ws).eof()) {
			message = "Введите два целых числа: строку и столбец; Q — выход.";
			return;
		}
		apply(row, column, message);
		return;
	}
	int choice = 0;
	const Robot& player = field_.robot(Robot::PlayerId);
	if (!parseNumber(input, choice) || choice < 1 || choice > player.abilityCount()) {
		message = "Неверный номер способности; Q — выход.";
		return;
	}
	selected_ = choice - 1;
	if (AbilityCatalog::requiresTarget(player.ability(selected_))) {
		mode_ = Mode::Target;
		message = "Выберите цель.";
	} else {
		apply(0, 0, message);
	}
}

std::string AbilityMenu::prompt() const {
	if (mode_ == Mode::Selection) {
		return "Введите номер способности из списка; Q — выход.";
	}
	if (mode_ == Mode::Target) {
		return "Введите строку и столбец через пробел. Нумерация с 0; Q — выход.";
	}
	if (mode_ == Mode::Command) {
		return "";
	}
	std::string text = mode_ == Mode::Initial ? "Выберите начальную способность:\n"
		: "Награда за ранг " + std::to_string(awardedRank_ + 1) + ":\n";
	for (int choice = 1; choice <= AbilityCatalog::Count; ++choice) {
		auto ability = AbilityCatalog::create(choice);
		int index = AbilityCatalog::find(field_.robot(Robot::PlayerId), ability->kind());
		text += std::to_string(choice) + " — " + ability->name() + ": ";
		if (index < 0) {
			text += "получить, " + std::to_string(ability->energyCost()) + " энергии";
		} else {
			text += field_.robot(Robot::PlayerId).ability(index).canUpgrade() ? "улучшить" : "предел улучшений";
		}
		text += '\n';
	}
	return text + "Введите номер; Q — выход.";
}
