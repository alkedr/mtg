#include "magic.hpp"

#include <boost/preprocessor.hpp>
#include <algorithm>
#include <iostream>



void Game::ManaPool::add(Color color, short int count) {
	s.emplace_back(color, count);
}
void Game::ManaPool::subtract(const Cost & cost) {
	short int colorlessCount = 0;
	for (const auto & pair : cost) {
		if (pair.first == Color::COLORLESS) {
			colorlessCount += pair.second;
		} else {
			auto it = std::find_if(std::begin(s), std::end(s), [&](const std::pair<Color, short int> & p) { return p.first == pair.first; } );
			if (it == s.end()) throw ENotEnoughMana();
			it->second -= pair.second;
			if (it->second < 0) throw ENotEnoughMana();
		}
	}
	for (auto & pair : s) {
		int a = std::min(colorlessCount, pair.second);
		pair.second -= a;
		colorlessCount -= a;
	}
	if (colorlessCount > 0) throw ENotEnoughMana();
}


class Game::Impl {
public:

	class Attack {
	public:
		CardInGameId attacker;
		Target target;

		Attack(CardInGameId _attacker, Target _target) : attacker(_attacker), target(_target) {}
	};

	class Block {
	public:
		CardInGameId blocker;
		Target target;

		Block(CardInGameId _blocker, Target _target) : blocker(_blocker), target(_target) {}
	};


	Turn turn;
	std::vector<Player> players_;    // TODO: vector<unique_ptr>  or const
	std::vector<std::unique_ptr<Card>> cards;  // all cards_ allocated here,  CardInGameId - index in this vector  DO NOT ERASE! DO NOT FREE! DO NOT NULL!
	Stack stack;
	Effects effects;

	unsigned char maxLandsPerTurn_ = 1;     // TODO: remove
	unsigned char landsPlayedThisTurn_ = 0; // TODO: remove

	std::vector<Attack> attackers;
	std::vector<Block> blockers;

	Impl() {}
	Impl(const Impl & other)
	: turn(other.turn)
	, players_(other.players_)
	, maxLandsPerTurn_(other.maxLandsPerTurn_)
	, landsPlayedThisTurn_(other.landsPlayedThisTurn_)
	, attackers(other.attackers)
	, blockers(other.blockers)
	{
		for (const auto & pCard : other.cards) cards.emplace_back(pCard->clone());
		for (const auto & pEffect : other.effects) effects.emplace_back(pEffect->clone());
		for (const auto & effectsOnStack : other.stack) {
			stack.emplace_back();
			for (const auto & pEffect : effectsOnStack) stack.back().emplace_back(pEffect->clone());
		}
	}

	
	Player & player(PlayerId id) { return players_.at(id-1); }
	const Player & player(PlayerId id) const { return players_.at(id-1); }


	PlayerId addPlayer(std::string name, std::vector<Card::Id> library) {
		players_.emplace_back(name, library);
		return (PlayerId)players_.size();
	}

	bool allPlayersPassed() const {
		return std::all_of(
			std::begin(players_),
			std::end(players_),
			[&](const Player & p) {
				return p.passed;
			}
		);
	}

	void startPhase(Turn::Phase phase) {
		switch (phase) {
			case (Turn::Phase::UNTAP): {
				for (std::unique_ptr<Card> & pCard : cards) {
					if (pCard->ownerId() == turn.activePlayerId) pCard->untap();
				}
				goToNextPhase();
				break;
			}
			case (Turn::Phase::UPKEEP): {
				break;
			}
			case (Turn::Phase::DRAW_CARD): {
				drawCard(turn.activePlayerId);
				break;
			}
			case (Turn::Phase::FIRST_MAIN): {
				break;
			}
			case (Turn::Phase::COMBAT_BEGIN): {
				break;
			}
			case (Turn::Phase::COMBAT_ATTACK): {
				break;
			}
			case (Turn::Phase::COMBAT_BLOCK): {
				break;
			}
			case (Turn::Phase::COMBAT_DAMAGE): {
				break;
			}
			case (Turn::Phase::COMBAT_END): {
				break;
			}
			case (Turn::Phase::SECOND_MAIN): {
				break;
			}
			case (Turn::Phase::END): {
				break;
			}
			case (Turn::Phase::CLEANUP): {
				break;
			}
		}
	}

	void endPhase(Turn::Phase phase) {
		switch (phase) {
			case (Turn::Phase::UNTAP): {
				break;
			}
			case (Turn::Phase::UPKEEP): {
				break;
			}
			case (Turn::Phase::DRAW_CARD): {
				break;
			}
			case (Turn::Phase::FIRST_MAIN): {
				break;
			}
			case (Turn::Phase::COMBAT_BEGIN): {
				break;
			}
			case (Turn::Phase::COMBAT_ATTACK): {
				break;
			}
			case (Turn::Phase::COMBAT_BLOCK): {
				break;
			}
			case (Turn::Phase::COMBAT_DAMAGE): {
				break;
			}
			case (Turn::Phase::COMBAT_END): {
				break;
			}
			case (Turn::Phase::SECOND_MAIN): {
				break;
			}
			case (Turn::Phase::END): {
				break;
			}
			case (Turn::Phase::CLEANUP): {
				landsPlayedThisTurn_ = 0;
				break;
			}
		}
	}

	void goToNextPhase() {
		endPhase(turn.phase);
		turn.phase = (Turn::Phase)((int)turn.phase << 1);
		if (turn.phase > Turn::Phase::CLEANUP) {
			turn.phase = Turn::Phase::UNTAP;
			turn.activePlayerId = 1 - turn.activePlayerId;
			turn.priorityPlayerId = turn.activePlayerId;
		}
		startPhase(turn.phase);
	}

	void lose(PlayerId playerId) {
		player(playerId).loser = true;
	}

	void drawCard(PlayerId playerId) {
		if (player(playerId).library.empty()) {
			lose(playerId);
		} else {
			Card::Id cardId = player(playerId).library.back();
			player(playerId).library.pop_back();
			cards.push_back( newCard(cardId, playerId) );
			cards.back()->setPosition(Card::Position::HAND);
		}
	}

	void clearPlayerPassFlag() {
		for (Player & player : players_) player.passed = false;
	}

	void addEffect(std::unique_ptr<Effect> pEffect) {
		pEffect->resolve(*this);
		effects.push_back(std::move(pEffect));
	}

	void addEffects(Effects & newEffects) {
		for (auto & pEffect : newEffects) addEffect(std::move(pEffect));
	}



	void start(PlayerId firstPlayer) {
		turn.activePlayerId = firstPlayer;
		turn.priorityPlayerId = turn.activePlayerId;
		for (PlayerId i = 1; i <= players_.size(); i++) {
			for (int j = 0; j < 7; j++) drawCard(i);
		}
	}

	void playCardFromHand(PlayerId playerId, CardInGameId cardInGameId) {
		if (cards.at(cardInGameId)->ownerId() != playerId) throw EWrongCardOwner();
		cards.at(cardInGameId)->playFromHand(*this, cardInGameId);
		clearPlayerPassFlag();
	}

	void activateAbility(PlayerId playerId, CardInGameId cardInGameId) {
		if (cards.at(cardInGameId)->ownerId() != playerId) throw EWrongCardOwner();
		cards.at(cardInGameId)->activateAbility(*this, cardInGameId);
		clearPlayerPassFlag();
	}

	void pass(PlayerId playerId) {
		if (turn.priorityPlayerId != playerId) throw ENotYourPriority();
		player(playerId).passed = true;
		if (allPlayersPassed()) {
			if (stack.empty()) {
				goToNextPhase();
			} else {
				addEffects(stack.back());
				stack.pop_back();
			}
			clearPlayerPassFlag();
			turn.priorityPlayerId = turn.activePlayerId;
		} else {
			turn.priorityPlayerId = 3 - turn.priorityPlayerId;
		}
	}

	void declareAttacker(PlayerId playerId, CardInGameId cardInGameId, Target target) {
		if (turn.activePlayerId != playerId) throw ENotYourTurn();
		if (turn.priorityPlayerId != playerId) throw ENotYourPriority();
		if (turn.phase != Turn::Phase::COMBAT_ATTACK) throw EWrongPhase();
		if (cards.at(cardInGameId)->position() != Card::Position::BATTLEFIELD) throw EWrongZone();
		if (cards.at(cardInGameId)->type() != Card::Type::CREATURE) throw EWrongCardType();
		attackers.emplace_back(cardInGameId, target);
	}

	void declareBlocker(PlayerId playerId, CardInGameId cardInGameId, Target target) {
		if (turn.activePlayerId != playerId) throw ENotYourTurn();
		if (turn.priorityPlayerId != playerId) throw ENotYourPriority();
		if (turn.phase != Turn::Phase::COMBAT_BLOCK) throw EWrongPhase();
		if (cards.at(cardInGameId)->position() != Card::Position::BATTLEFIELD) throw EWrongZone();
		if (cards.at(cardInGameId)->type() != Card::Type::CREATURE) throw EWrongCardType();
		blockers.emplace_back(cardInGameId, target);
	}


};


Game::Effect::Effect(CardInGameId source) : source_(source) {
}
Game::Effect::~Effect() {
}
void Game::Effect::resolve(Impl & impl) {
	onResolve(impl);
}



#define STRINGIZE(arg)  STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOR_EACH_1(what, x) what(x)
#define FOR_EACH_2(what, x, ...)  what(x) FOR_EACH_1(what, __VA_ARGS__)
#define FOR_EACH_3(what, x, ...)  what(x) FOR_EACH_2(what, __VA_ARGS__)
#define FOR_EACH_4(what, x, ...)  what(x) FOR_EACH_3(what, __VA_ARGS__)
#define FOR_EACH_5(what, x, ...)  what(x) FOR_EACH_4(what, __VA_ARGS__)
#define FOR_EACH_6(what, x, ...)  what(x) FOR_EACH_5(what, __VA_ARGS__)
#define FOR_EACH_7(what, x, ...)  what(x) FOR_EACH_6(what, __VA_ARGS__)
#define FOR_EACH_8(what, x, ...)  what(x) FOR_EACH_7(what, __VA_ARGS__)

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, x, ...) CONCATENATE(FOR_EACH_, N)(what, x, __VA_ARGS__)
#define FOR_EACH(what, x, ...) FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), what, x, __VA_ARGS__)



#define __EFFECT_PARAMETER_FIELD(TYPE, NAME) private: TYPE NAME ## _;
#define __EFFECT_PARAMETER_GETTER(TYPE, NAME) public: TYPE NAME() const { return NAME ## _; }
#define __EFFECT_PARAMETER_CONSTRUCTOR(TYPE, NAME) , TYPE NAME
#define __EFFECT_PARAMETER_INITIALIZER(TYPE, NAME) , NAME ## _(NAME)
#define _EFFECT_PARAMETER_FIELD(ARGS) __EFFECT_PARAMETER_FIELD ARGS
#define _EFFECT_PARAMETER_GETTER(ARGS) __EFFECT_PARAMETER_GETTER ARGS
#define _EFFECT_PARAMETER_CONSTRUCTOR(ARGS) __EFFECT_PARAMETER_CONSTRUCTOR ARGS
#define _EFFECT_PARAMETER_INITIALIZER(ARGS) __EFFECT_PARAMETER_INITIALIZER ARGS


#define EFFECT_BEGIN(NAME, PARENT, ...)                                                               \
	class NAME ## Effect : public PARENT {                                                              \
		FOR_EACH(_EFFECT_PARAMETER_FIELD, __VA_ARGS__)                                                    \
	public:                                                                                             \
		NAME ## Effect(Game::CardInGameId source FOR_EACH(_EFFECT_PARAMETER_CONSTRUCTOR, __VA_ARGS__))    \
		 : Game::Effect(source) FOR_EACH(_EFFECT_PARAMETER_INITIALIZER, __VA_ARGS__) {}                   \
		FOR_EACH(_EFFECT_PARAMETER_GETTER, __VA_ARGS__)                                                   \
		virtual Effect * clone() const override { return new NAME ## Effect(*this); }

#define EFFECT_END };

#define ON_RESOLVE public: virtual void onResolve(Game::Impl & impl) override




EFFECT_BEGIN(AddMana, Game::Effect,
	(Game::PlayerId, targetPlayerId),
	(Color, color)
)
	ON_RESOLVE {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.player(targetPlayerId_).manaPool.add(color_);
	}
EFFECT_END

EFFECT_BEGIN(TakeMana, Game::Effect,
	(Game::PlayerId, targetPlayerId),
	(Game::Cost, cost)
)
	ON_RESOLVE {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.player(targetPlayerId_).manaPool.subtract(cost_);
	}
EFFECT_END


EFFECT_BEGIN(MoveCard, Game::Effect,
	(Game::CardInGameId, targetCardInGameId),
	(Game::Card::Position, position)
)
	ON_RESOLVE {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.cards.at(targetCardInGameId_)->setPosition(position_);
	}
EFFECT_END

EFFECT_BEGIN(SetTap, Game::Effect,
	(Game::CardInGameId, targetCardInGameId),
	(bool, value)
)
	ON_RESOLVE {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.cards.at(targetCardInGameId_)->setTapped(value());
	}
EFFECT_END




Game::Game() : pimpl(new Impl) {
}
Game::Game(const Game & other) : pimpl(new Impl(*other.pimpl)) {
}
Game::~Game() {
}

Game::Player & Game::player(PlayerId id) { return pimpl->player(id); }
const Game::Player & Game::player(PlayerId id) const { return pimpl->player(id); }
std::vector<Game::Player> & Game::players() { return pimpl->players_; }
const std::vector<Game::Player> & Game::players() const { return pimpl->players_; }

Game::Card & Game::card(CardInGameId cardInGameId) { return *pimpl->cards.at(cardInGameId); }
std::vector<std::unique_ptr<Game::Card>> & Game::cards() { return pimpl->cards; }
const std::vector<std::unique_ptr<Game::Card>> & Game::cards() const { return pimpl->cards; }

Game::Turn & Game::turn() { return pimpl->turn; }
const Game::Turn & Game::turn() const { return pimpl->turn; }

Game::PlayerId Game::addPlayer(std::string name, std::vector<Card::Id> library) {
	return pimpl->addPlayer(name, library);
}

void Game::start(PlayerId firstPlayer) {
	pimpl->start(firstPlayer);
}
void Game::playCardFromHand(PlayerId playerId, CardInGameId cardInGameId) {
	pimpl->playCardFromHand(playerId, cardInGameId);
}
void Game::activateAbility(PlayerId playerId, CardInGameId cardInGameId) {
	pimpl->activateAbility(playerId, cardInGameId);
}
void Game::pass(PlayerId playerId) {
	pimpl->pass(playerId);
}





template <Game::Card::Id _id> class CardHelper : public Game::Card {
	static const Info info_;
public:
	virtual const Info & info() const {
		return info_;
	}

	CardHelper(Game::PlayerId ownerId)
	: Card(ownerId) {}

	virtual Card * clone() const override { return new CardHelper<_id>(*this); }
};

template<Game::Card::Id _id> const Game::Card::Info CardHelper<_id>::info_ {
	.id = _id,
	.name = "Unknown Card",
	.description = "Unknown"
};


#define CARD_BEGIN(BASE, ID, NAME, DESCRIPTION)                                                              \
template<> class CardHelper<ID> : public BASE {                                                              \
public:                                                                                                      \
	CardHelper<ID>(Game::PlayerId ownerId)                                                                     \
	: BASE(ownerId) {}                                                                                         \
	virtual const Info & info() const override {                                                               \
		static Info info_ {                                                                                      \
			.id = ID,                                                                                              \
			.name = NAME,                                                                                          \
			.description = DESCRIPTION                                                                             \
		};                                                                                                       \
		return info_;                                                                                            \
	}                                                                                                          \
	virtual Card * clone() const override { return new CardHelper<ID>(*this); }

#define CARD_END  };




#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0 (EVAL0 (EVAL0 (__VA_ARGS__)))
#define EVAL2(...) EVAL1 (EVAL1 (EVAL1 (__VA_ARGS__)))
#define EVAL3(...) EVAL2 (EVAL2 (EVAL2 (__VA_ARGS__)))
#define EVAL4(...) EVAL3 (EVAL3 (EVAL3 (__VA_ARGS__)))
#define EVAL(...) EVAL4 (EVAL4 (EVAL4 (__VA_ARGS__)))

#define MAP_END(...)

#define MAP_OUT
#define MAP_GET_END() 0, MAP_END
#define MAP_NEXT0(item, next, ...) next MAP_OUT
#define MAP_NEXT1(item, next) MAP_NEXT0 (item, next, 0)
#define MAP_NEXT(item, next) MAP_NEXT1 (MAP_GET_END item, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT (peek, MAP1) (f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT (peek, MAP0) (f, peek, __VA_ARGS__)
#define MAP(f, ...) EVAL (MAP1 (f, __VA_ARGS__, (), 0))





#define OVERRIDE(RETURN_VALUE, FUNCTION_NAME, ATTRIBUTES, ...) virtual RETURN_VALUE FUNCTION_NAME(__VA_ARGS__) ATTRIBUTES override final

#define GETTER_TYPE(TYPE_NAME, PROPERTY_NAME) OVERRIDE(TYPE_NAME, PROPERTY_NAME, const, )
#define GETTER_TYPE_SIMPLE(TYPE_NAME, PROPERTY_NAME, VALUE) GETTER_TYPE(TYPE_NAME, PROPERTY_NAME) { return VALUE; }

#define COLOR(VALUE) GETTER_TYPE_SIMPLE(Color, color, VALUE)
#define POWER(VALUE) GETTER_TYPE_SIMPLE(Power, power, VALUE)
#define TOUGHNESS(VALUE) GETTER_TYPE_SIMPLE(Toughness, toughness, VALUE)

#define __EFFECT(X) effects.emplace_back(new X);

#define PUSH_EFFECTS(...) {                                                                \
	Game::Effects effects;                                                                 \
	MAP(__EFFECT, __VA_ARGS__);                                                            \
	impl.stack.push_back(std::move(effects));                                              \
}

#define CHECK_POSITION(VALUE, ALLOWED_VALUES) {                                            \
	if ((VALUE & ALLOWED_VALUES) == 0) {                                                   \
		throw EWrongPosition(VALUE, ALLOWED_VALUES);                                       \
	}                                                                                      \
}



class Artifact : public Game::Card {
public:
	virtual Type type() const override { return Type::ARTIFACT; }
};
class Creature : public Game::Card {
public:
	typedef short Power;
	typedef short Toughness;

	virtual Type type() const override { return Type::CREATURE; }

	virtual Power power() const = 0;
	virtual Toughness toughness() const = 0;

	virtual Game::Cost cost() const = 0;

	virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) override {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		CHECK_POSITION(position(), Position::HAND);
		TakeManaEffect(myInGameId, ownerId(), cost()).resolve(impl);
		PUSH_EFFECTS(
			MoveCardEffect(myInGameId, myInGameId, Position::BATTLEFIELD)
		)
	}

	Creature(Game::PlayerId ownerId)
	: Card(ownerId) {}
};
class Enchantment : public Game::Card {
public:
	virtual Type type() const override { return Type::ENCHANTMENT; }
};
class Instant : public Game::Card {
public:
	virtual Type type() const override { return Type::INSTANT; }
};
class Land : public Game::Card {
public:
	virtual Type type() const override { return Type::LAND; }

	Land(Game::PlayerId ownerId)
	: Card(ownerId) {}
};
class Planeswalker : public Game::Card {
public:
	virtual Type type() const override { return Type::PLANESWALKER; }
};
class Sorcery : public Game::Card {
public:
	virtual Type type() const override { return Type::SORCERY; }
};

template<Color color_> class BasicLand : public Land {
public:
	BasicLand(Game::PlayerId ownerId)
	: Land(ownerId) {}

	virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) override {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		CHECK_POSITION(position(), Position::HAND);
		MoveCardEffect(myInGameId, myInGameId, Position::BATTLEFIELD).resolve(impl);
	}
	virtual void activateAbility(Game::Impl & impl, Game::CardInGameId myInGameId) override {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		CHECK_POSITION(position(), Position::BATTLEFIELD);
		if (tapped() != false) throw EAlreadyTapped();
		SetTapEffect(myInGameId, myInGameId, true).resolve(impl);
		AddManaEffect(myInGameId, ownerId(), color_).resolve(impl);
	}
};




CARD_BEGIN(BasicLand<Color::WHITE>, 1, "Plains",   "Plains")
CARD_END

CARD_BEGIN(BasicLand<Color::BLUE>,  2, "Island",   "Island")
CARD_END

CARD_BEGIN(BasicLand<Color::BLACK>, 3, "Swamp",    "Swamp")
CARD_END

CARD_BEGIN(BasicLand<Color::RED>,   4, "Mountain", "Mountain")
CARD_END

CARD_BEGIN(BasicLand<Color::GREEN>, 5, "Forest",   "Forest")
CARD_END




CARD_BEGIN(Creature, 6, "Grizzly Bears", "")
	POWER(2)
	TOUGHNESS(2)
	virtual Game::Cost cost() const {
		return Game::Cost {
			{ Color::COLORLESS, 1 },
			{ Color::GREEN, 1 }
		};
	}
CARD_END




#define CARD(Z, ID, DATA) case(DATA+ID): { return new CardHelper<DATA+ID>(ownerId); }

static Game::Card * newCardHelper(const Game::Card::Id cardId, Game::PlayerId ownerId) {
	switch (cardId) {
		BOOST_PP_REPEAT(255, CARD, 0)
		BOOST_PP_REPEAT(255, CARD, 255)
		default: { return new CardHelper<0>(ownerId); }
	}
}
std::unique_ptr<Game::Card> Game::newCard(const Game::Card::Id cardId, Game::PlayerId ownerId) {
	return std::move(std::unique_ptr<Game::Card>(newCardHelper(cardId, ownerId)));
}

