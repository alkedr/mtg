#include "magic.hpp"

#include <boost/preprocessor.hpp>




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
	std::vector<Player> players;    // TODO: vector<unique_ptr>  or const
	std::vector<std::unique_ptr<Card>> cards;  // all cards_ allocated here,  CardInGameId - index in this vector  DO NOT ERASE! DO NOT FREE! DO NOT NULL!
	Stack stack;
	Effects effects;

	unsigned char maxLandsPerTurn_ = 1;     // TODO: remove
	unsigned char landsPlayedThisTurn_ = 0; // TODO: remove

	std::vector<Attack> attackers;
	std::vector<Block> blockers;


	bool allPlayersPassed() const {
		return std::all_of(
			std::begin(players),
			std::end(players),
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
		turn.phase = (Turn::Phase)((int)turn.phase + 1);
		if (turn.phase > Turn::Phase::CLEANUP) {
			turn.phase = Turn::Phase::UNTAP;
			turn.activePlayerId = 1 - turn.activePlayerId;
			turn.priorityPlayerId = turn.activePlayerId;
		}
		startPhase(turn.phase);
	}

	void lose(PlayerId playerId) {
		players.at(playerId).loser = true;
	}

	void drawCard(PlayerId playerId) {
		if (players.at(playerId).library.empty()) {
			lose(playerId);
		} else {
			Card::Id cardId = players.at(playerId).library.back();
			players.at(playerId).library.pop_back();
			cards.push_back( newCard(cardId, playerId) );
			cards.back()->setPosition(Card::Position::HAND);
		}
	}

	void clearPlayerPassFlag() {
		for (Player & player : players) player.passed = false;
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
		for (PlayerId i = 0; i < players.size(); i++) {
			for (int j = 0; j < 7; j++) drawCard(i);
		}
	}

	void playCardFromHand(PlayerId playerId, CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (cards.at(cardInGameId)->ownerId() != playerId) throw EWrongCardOwner();
		cards.at(cardInGameId)->playFromHand(*this, cardInGameId);
		clearPlayerPassFlag();
	}

	void activateAbility(PlayerId playerId, CardInGameId cardInGameId) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (cards.at(cardInGameId)->ownerId() != playerId) throw EWrongCardOwner();
		cards.at(cardInGameId)->activateAbility(*this, cardInGameId);
		clearPlayerPassFlag();
	}

	void pass(PlayerId playerId) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (turn.priorityPlayerId != playerId) throw ENotYourPriority();
		players.at(playerId).passed = true;
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
			turn.priorityPlayerId = 1 - turn.priorityPlayerId;
		}
	}

	void declareAttacker(PlayerId playerId, CardInGameId cardInGameId, Target target) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (turn.activePlayerId != playerId) throw ENotYourTurn();
		if (turn.priorityPlayerId != playerId) throw ENotYourPriority();
		if (turn.phase != Turn::Phase::COMBAT_ATTACK) throw EWrongPhase();
		if (cards.at(cardInGameId)->position() != Card::Position::BATTLEFIELD) throw EWrongZone();
		if (cards.at(cardInGameId)->type() != Card::Type::CREATURE) throw EWrongCardType();
		attackers.emplace_back(cardInGameId, target);
	}

	void declareBlocker(PlayerId playerId, CardInGameId cardInGameId, Target target) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		if (turn.activePlayerId != playerId) throw ENotYourTurn();
		if (turn.priorityPlayerId != playerId) throw ENotYourPriority();
		if (turn.phase != Turn::Phase::COMBAT_BLOCK) throw EWrongPhase();
		if (cards.at(cardInGameId)->position() != Card::Position::BATTLEFIELD) throw EWrongZone();
		if (cards.at(cardInGameId)->type() != Card::Type::CREATURE) throw EWrongCardType();
		blockers.emplace_back(cardInGameId, target);
	}


};



Game::Effect::Effect(CardInGameId source) : source_(source) {}
Game::Effect::~Effect() {}

void Game::Effect::resolve(Impl & impl)
{
	onResolve(impl);
}




class AddManaEffect : public Game::Effect {
	Color color_;
	Game::PlayerId targetPlayerId_;
public:
	AddManaEffect(Game::CardInGameId source, Game::PlayerId targetPlayerId, Color color)
	 : Game::Effect(source)
	 , color_(color)
	 , targetPlayerId_(targetPlayerId) {}

	virtual void onResolve(Game::Impl & impl) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.players.at(targetPlayerId_).manaPool.add(color_);
	}
};

class MoveCardEffect : public Game::Effect {
	Game::CardInGameId targetCardInGameId_;
	Game::Card::Position position_;
public:
	MoveCardEffect(Game::CardInGameId source, Game::CardInGameId targetCardInGameId, Game::Card::Position position)
	 : Game::Effect(source)
	 , targetCardInGameId_(targetCardInGameId)
	 , position_(position)
	{}

	virtual void onResolve(Game::Impl & impl) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.cards.at(targetCardInGameId_)->setPosition(position_);
	}
};

class SetTapEffect : public Game::Effect {
	Game::CardInGameId targetCardInGameId_;
	bool value_;
public:
	SetTapEffect(Game::CardInGameId source, Game::CardInGameId targetCardInGameId, bool value)
	 : Game::Effect(source)
	 , targetCardInGameId_(targetCardInGameId)
	 , value_(value)
	{}

	virtual void onResolve(Game::Impl & impl) {
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		impl.cards.at(targetCardInGameId_)->setTapped(value_);
	}
};




Game::Game() : pimpl(new Impl) {
}

Game::~Game() {
}

Game::Player & Game::player(PlayerId id) { return pimpl->players.at(id); }
const Game::Player & Game::player(PlayerId id) const { return pimpl->players.at(id); }
std::vector<Game::Player> & Game::players() { return pimpl->players; }

Game::Card & Game::card(CardInGameId cardInGameId) { return *pimpl->cards.at(cardInGameId); }
std::vector<std::unique_ptr<Game::Card>> & Game::cards() { return pimpl->cards; }
const std::vector<std::unique_ptr<Game::Card>> & Game::cards() const { return pimpl->cards; }

Game::Turn & Game::turn() { return pimpl->turn; }
const Game::Turn & Game::turn() const { return pimpl->turn; }

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

	public:

		virtual Type type() const override { return Type::LAND; }

		virtual Id id() const override { return _id; }
		virtual const char * name() const { return "Unknown Card"; }
		virtual const char * description() const { return "Unknown"; }

		virtual const std::string getImageName() const { return ""; }   // FIXME: image for unknown card

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) override {}

		CardHelper(Game::PlayerId ownerId)
		: Card(ownerId) {}
	};


	#define CARD_BEGIN(BASE, ID, NAME, DESCRIPTION)                                                              \
	template<> class CardHelper<ID> : public BASE {                                                              \
	public:                                                                                                      \
		CardHelper<ID>(Game::PlayerId & ownerId)                                                                   \
		: BASE(ownerId) {}                                                                                         \
		GETTER_TYPE_SIMPLE(Id, id, ID)                                                                             \
		GETTER_TYPE_SIMPLE(const char *, name, NAME)                                                               \
		GETTER_TYPE_SIMPLE(const char *, description, DESCRIPTION)

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

	#define EFFECTS(...)                                                                   \
		virtual Game::Effects effects(Game::CardInGameId myInGameId) const override {        \
			Game::Effects effects;                                                             \
			MAP(__EFFECT, __VA_ARGS__);                                                        \
			return std::move(effects);                                                         \
		}

	#define PUSH_EFFECTS(...)                                                              \
		{                                                                                    \
			Game::Effects effects;                                                             \
			MAP(__EFFECT, __VA_ARGS__);                                                        \
			impl.stack.push_back(std::move(effects));                                                \
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

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId)
		{
			/*std::cout << __PRETTY_FUNCTION__ << std::endl;
			owner().manaPool.subtract(getCost());
			game.stack().push(*this, &Card::moveToBattlefield);
			position = Position::STACK;*/
		}


		Creature(Game::PlayerId ownerId)
		: Card(ownerId) {}
	};

	class Enchantment : public Game::Card {

	public:

		virtual Type type() const override { return Type::ENCHANTMENT; }

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId)
		{
			//game.stack.push(new PlayCardEffect(*this));
		}
	};

	class Instant : public Game::Card {

	public:

		virtual Type type() const override { return Type::INSTANT; }

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId)
		{
			//game.stack.push(new PlayCardEffect(*this));
		}
	};

	class Land : public Game::Card {

	public:

		virtual Type type() const override { return Type::LAND; }

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) {
			/*std::cout << __PRETTY_FUNCTION__ << std::endl;
			if (position != Card::Position::HAND) throw EWrongZone();
			if (ownerId != game.turn().activePlayerId) throw ENotYourTurn();
			if ((game.turn().phase != Game::Turn::Phase::FIRST_MAIN) && (game.turn().phase != Game::Turn::Phase::SECOND_MAIN)) throw EWrongPhase();
			if (game.landsPlayedThisTurn_ >= game.maxLandsPerTurn_) throw ETooMuchLandsPerTurn();
			game.landsPlayedThisTurn_++;
			position = Position::BATTLEFIELD;*/
		}

		// virtual void activateAbility() override {
			/*std::cout << __PRETTY_FUNCTION__ << std::endl;
			tapped_ = true;
			afterTap();*/
		// }


		Land(Game::PlayerId ownerId)
		: Card(ownerId) {}
	};

	class Planeswalker : public Game::Card {

	public:

		virtual Type type() const override { return Type::PLANESWALKER; }

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) {
			//game.stack.push(new PlayCardEffect(*this));
		}
	};

	class Sorcery : public Game::Card {

	public:

		virtual Type type() const override { return Type::SORCERY; }

		virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) {
			//game.stack.push(new PlayCardEffect(*this));
		}
	};



template<Color color_> class BasicLand : public Land {
public:
	BasicLand(Game::PlayerId ownerId)
	: Land(ownerId) {}

	virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) override {
		if (position() != Position::HAND) throw EWrongZone();
		MoveCardEffect(myInGameId, myInGameId, Position::BATTLEFIELD).resolve(impl);
	}
	virtual void activateAbility(Game::Impl & impl, Game::CardInGameId myInGameId) override {
		if (position() != Position::BATTLEFIELD) throw EWrongZone();
		if (tapped() != false) throw EAlreadyTapped();
		SetTapEffect(myInGameId, myInGameId, true).resolve(impl);
		AddManaEffect(myInGameId, ownerId(), color_).resolve(impl);
	}
};




CARD_BEGIN(BasicLand<Color::WHITE>,  1, "Plains",   "Plains")
CARD_END

CARD_BEGIN(BasicLand<Color::BLUE>,   2, "Island",   "Island")
CARD_END

CARD_BEGIN(BasicLand<Color::BLACK>,  3, "Swamp",    "Swamp")
CARD_END

CARD_BEGIN(BasicLand<Color::RED>,    4, "Mountain", "Mountain")
CARD_END

CARD_BEGIN(BasicLand<Color::GREEN>,  5, "Forest",   "Forest")
CARD_END




CARD_BEGIN(Creature,   6, "Grizzly Bears", "")
	POWER(2)
	TOUGHNESS(2)
	virtual void playFromHand(Game::Impl & impl, Game::CardInGameId myInGameId) override {
		if (position() != Position::HAND) throw EWrongZone();
		PUSH_EFFECTS(
			MoveCardEffect(myInGameId, myInGameId, Position::BATTLEFIELD)
		)
	}
	virtual Game::Cost cost() const {
		return Game::Cost {
			{ Color::COLORLESS, 1 },
			{ Color::GREEN, 1 }
		};
	}
	EFFECTS(
		MoveCardEffect(myInGameId, myInGameId, Position::BATTLEFIELD)
	)
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

#undef CARD
